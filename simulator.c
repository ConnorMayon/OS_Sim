/*
 * Author: Connor Mayon
*/

#include "simulator.h"

// Main function to run simulator
void runSim( ConfigDataType *configDataPtr, OpCodeType *metaDataPtr )
   {
    // Initialize variables
    const char WRITE_ONLY_CHAR[] = "w";

    int runTime, numProcesses;
    int *runTimePtr;
    int intArgs[2];
    pthread_t threadNum;
    bool ioInUse = false;
    bool logToMonitor = true, logToFile = true, interupt = false, schedulePreemtive = false;
    char *timeStr;
    ProcessControlBoard *PCBHead;
    ProcessControlBoard *tempPCB;
    memAllocation *memAllocHead = NULL;
    LogFileLine *logFileHead;
    LogFileLine *currLogLine;

    PCBHead = (ProcessControlBoard*) malloc( sizeof( ProcessControlBoard ) );
    createPCBLinkedList(metaDataPtr, PCBHead);

    tempPCB = PCBHead;
    numProcesses = 0;
    while( tempPCB != NULL )
    {
     calcRemProcTime (tempPCB, configDataPtr->procCycleRate, configDataPtr->ioCycleRate);
     numProcesses++;
     tempPCB->usingIO = false;
     tempPCB = tempPCB->nextPCB;
    }

    if (configDataPtr->cpuSchedCode == CPU_SCHED_FCFS_P_CODE ||
        configDataPtr->cpuSchedCode == CPU_SCHED_SRTF_P_CODE ||
        configDataPtr->cpuSchedCode == CPU_SCHED_RR_P_CODE)
       {
        schedulePreemtive = true;
       }

    setProcessPriorities( PCBHead, configDataPtr->cpuSchedCode );
    
    timeStr = (char*) malloc( sizeof(char) * MIN_STR_LEN );

    if( configDataPtr->logToCode == LOGTO_FILE_CODE )
       {
        logToMonitor = false;
       }
    else if( configDataPtr->logToCode == LOGTO_MONITOR_CODE )
       {
        logToFile = false;
       }

    if( logToFile )
       {
        logFileHead = (LogFileLine*) malloc( sizeof( LogFileLine ) );
        currLogLine = writeLogFileHeader( logFileHead, configDataPtr );
       }

    // OS Start message
    printf( "Simulator Run\n" );
    printf( "-------------\n\n" );

    if( !logToMonitor )
       {
        printf( "Simulator running with output to file" );
       }

    // Start timer
    accessTimer( ZERO_TIMER, timeStr );
    if( logToMonitor )
       {
        printf( "%s, OS: Simulator start\n", timeStr );
       }
    if( logToFile )
       {
        sprintf( currLogLine->text, "%s, OS: Simulator start\n", timeStr );
        currLogLine->nextLine = (LogFileLine*) malloc( sizeof( LogFileLine ) );
        currLogLine = currLogLine->nextLine;
       }
    
    // Set all states to ready
    tempPCB = PCBHead;
    while( tempPCB != NULL )
       {
        accessTimer(LAP_TIMER, timeStr);
        tempPCB->processState = READY_STATE;
        
        intArgs[0] = NEW_STATE;
        intArgs[1] = READY_STATE;
        if( logToMonitor )
           {
            printToMonitor( STATE_CHANGE, timeStr, tempPCB->pid,
                intArgs, NULL_CHAR );
           }
        if( logToFile )
           {
            currLogLine = printToLogFile( currLogLine, STATE_CHANGE, timeStr,
                             tempPCB->pid, intArgs, NULL_CHAR );
           }

        tempPCB = tempPCB->nextPCB;
       }

    printf("\n");

    // Main algorithm
    while( !allProcessesExited( PCBHead ) )
       {
        // Iterate through PCBs
        if( tempPCB == NULL )
           {
            tempPCB = PCBHead;
           }

        if( tempPCB->schedPriority == 0 )
           {
            OpCodeType* currentInstruction = tempPCB->programCounter;
            tempPCB->remInstructCycles = currentInstruction->intArg2;
            bool isSegFault = false;
            accessTimer( LAP_TIMER, timeStr );
            tempPCB->processState = RUNNING_STATE;
            
            intArgs[0] = tempPCB->processtimeRemaining;
            if( logToMonitor )
               {
                printToMonitor( PROCESS_SELECTED, timeStr, tempPCB->pid,
                                    intArgs, NULL_CHAR );
               }
            if( logToFile )
               {
                currLogLine = printToLogFile( currLogLine, PROCESS_SELECTED,
                                  timeStr, tempPCB->pid, intArgs, NULL_CHAR );
               }

            // Iterate through meta data until end of process
            while((compareString(currentInstruction->command, "app") != STR_EQ)
               && (compareString(currentInstruction->strArg1, "end") != STR_EQ)
               && !interupt )
               {
                // if io
                if( compareString( currentInstruction->command, "dev" )
                                                                == STR_EQ && !ioInUse )
                   {
                    tempPCB->remInstructCycles = 0;
                    int ioType[2];

                    // check for input or output
                    if( compareString( currentInstruction->inOutArg, "in" )
                                                              == STR_EQ )
                       {
                        ioType[0] = IO_IN_START;
                        ioType[1] = IO_IN_END;
                       }
                    else
                       {
                        ioType[0] = IO_OUT_START;
                        ioType[1] = IO_OUT_END;
                       }

                    accessTimer( LAP_TIMER, timeStr );
                    if( logToMonitor )
                       {
                        printToMonitor( ioType[0], timeStr, tempPCB->pid,
                                    intArgs, currentInstruction->strArg1 );
                       }
                    if( logToFile )
                       {
                        currLogLine = printToLogFile( currLogLine, ioType[0], 
                                          timeStr, tempPCB->pid, intArgs, 
                                             currentInstruction->strArg1 );
                       }

                    runTime = currentInstruction->intArg2
                        * configDataPtr->ioCycleRate;
                    // Simulate io running
                    if( schedulePreemtive )
                       {
                        tempPCB->usingIO = true;
                        ioInUse = true;
                        ThreadArgs *ta = (ThreadArgs *)malloc(sizeof(ThreadArgs));
                        ta->isFCFSP = configDataPtr->cpuSchedCode == CPU_SCHED_FCFS_P_CODE;
                        ta->milliSeconds = runTime;
                        ta->numProcesses = numProcesses;
                        ta->PCB = tempPCB;
                        ta->timeStr = timeStr;
                        ta->ioType = ioType;
                        ta->currLine = currLogLine;
                        ta->ioInUse = &ioInUse;
                        ta->interupt = &interupt;

                        pthread_create(&threadNum, NULL, runTimerPreemptive,
                            (void*)ta);
                       }
                    else
                       {
                        runTimePtr = &runTime;
                        pthread_create(&threadNum, NULL, runTimerThread,
                            (void*)runTimePtr);

                        pthread_join(threadNum, NULL);
                        tempPCB->processtimeRemaining -= runTime;

                        accessTimer( LAP_TIMER, timeStr );
                        if( logToMonitor )
                           {
                            printToMonitor(ioType[1], timeStr, tempPCB->pid,
                                intArgs, currentInstruction->strArg1);
                           }
                        if( logToFile )
                           {
                            currLogLine = printToLogFile(currLogLine, ioType[1],
                                timeStr, tempPCB->pid, intArgs,
                                currentInstruction->strArg1);
                           }
                       }
                   }

                else if( compareString( currentInstruction->command, "mem" ) 
                                                            == STR_EQ )
                   {
                    tempPCB->remInstructCycles = 0;
                    // Handle mem allocation
                    if (compareString(currentInstruction->strArg1, "allocate") 
                                                                 == STR_EQ)
                       {
                        accessTimer(LAP_TIMER, timeStr);
                        intArgs[0] = currentInstruction->intArg2;
                        intArgs[1] = currentInstruction->intArg3;

                        if( logToMonitor )
                           {
                            printToMonitor( MEM_ALLOC, timeStr, tempPCB->pid,
                                               intArgs, NULL_CHAR );
                           }
                        if( logToFile )
                           {
                            currLogLine = printToLogFile( currLogLine, 
                                    MEM_ALLOC, timeStr, tempPCB->pid,
                                               intArgs, NULL_CHAR );
                           }
                        
                        // Check if memory has not been allocated
                        if( !isAlreadyAllocated( memAllocHead, 
                                                 currentInstruction->intArg2, 
                                                 currentInstruction->intArg3 ) )
                           {
                            memAllocHead = allocateMem( memAllocHead, 
                                         currentInstruction->intArg2,
                                currentInstruction->intArg3, tempPCB->pid );

                            if( logToMonitor )
                               {
                                printToMonitor( MEM_ALLOC_SUCC, timeStr, 
                                          tempPCB->pid, intArgs, NULL_CHAR );

                                printMemStatus( memAllocHead );
                               }
                            if( logToFile )
                               {
                                currLogLine = printToLogFile( currLogLine, 
                                                     MEM_ALLOC_SUCC, timeStr,
                                                      tempPCB->pid, intArgs, 
                                                              NULL_CHAR );
                               }
                           }

                        else
                           {
                            accessTimer( LAP_TIMER, timeStr );
                            if( logToMonitor )
                               {
                                printToMonitor( MEM_ALLOC_FAIL, timeStr, 
                                       tempPCB->pid, intArgs, NULL_CHAR );

                                printMemStatus( memAllocHead );
                               }
                            if( logToFile )
                               {
                                currLogLine = printToLogFile( currLogLine,
                                              MEM_ALLOC_FAIL, timeStr,
                                               tempPCB->pid, intArgs,
                                                   NULL_CHAR );
                               }

                            // End process early by skipping to end 
                            isSegFault = true;
                            currentInstruction =
                                        endProcessEarly( currentInstruction );
                            tempPCB->programCounter = currentInstruction;

                            accessTimer( LAP_TIMER, timeStr );
                            if( logToMonitor )
                               {
                                printToMonitor( OS_SEG_FAULT, timeStr, 
                                     tempPCB->pid, intArgs, NULL_CHAR );
                               }
                            if( logToFile )
                               {
                                currLogLine = printToLogFile( currLogLine, 
                                                OS_SEG_FAULT,
                                                timeStr,tempPCB->pid,
                                                intArgs, NULL_CHAR );
                               }
                            
                           }
                        
                       }
                    // Otherwise access memory
                    else
                       {
                        accessTimer( LAP_TIMER, timeStr );
                        intArgs[0] = currentInstruction->intArg2;
                        intArgs[1] = currentInstruction->intArg3;
                        if( logToMonitor )
                           {
                            printToMonitor( MEM_ACCESS, timeStr, tempPCB->pid,
                                                 intArgs, NULL_CHAR );
                           }
                        if( logToFile )
                           {
                            currLogLine = printToLogFile( currLogLine, 
                                                  MEM_ACCESS, timeStr,
                                           tempPCB->pid, intArgs, NULL_CHAR );
                           }
                        // Check if memory has been allocated to the right process
                        if( accessMem( memAllocHead, 
                                     currentInstruction->intArg2,
                                   currentInstruction->intArg3, tempPCB->pid ) )
                           {
                            if( logToMonitor )
                               {
                                printToMonitor( MEM_ACCESS_SUCC, timeStr, 
                                              tempPCB->pid, intArgs, 
                                                       NULL_CHAR );
                                printMemStatus( memAllocHead );
                               }
                            if( logToFile )
                               {
                                currLogLine = printToLogFile( currLogLine, 
                                                 MEM_ACCESS_SUCC, timeStr,
                                                 tempPCB->pid, intArgs,
                                                           NULL_CHAR );
                               }
                           }
                        else
                           {
                            if( logToMonitor )
                               {
                                printToMonitor( MEM_ACCESS_FAIL, timeStr,
                                             tempPCB->pid, intArgs,
                                                       NULL_CHAR );
                                printMemStatus( memAllocHead );
                               }
                            if( logToFile )
                               {
                                currLogLine = printToLogFile( currLogLine, 
                                                           MEM_ACCESS_FAIL, 
                                                        timeStr, tempPCB->pid, 
                                                        intArgs, NULL_CHAR );
                               }
                           }
                       }
                   }

                // if cpu process, wait
                else if( compareString(currentInstruction->command, "cpu") == STR_EQ )
                   {
                    int quantumCyclesRem = configDataPtr->quantumCycles;
                    accessTimer(LAP_TIMER, timeStr);
                    if( logToMonitor )
                       {
                        printToMonitor(CPU_START, timeStr, tempPCB->pid,
                            intArgs, NULL_CHAR);
                       }
                    if( logToFile )
                       {
                        currLogLine = printToLogFile(currLogLine, CPU_START,
                            timeStr, tempPCB->pid, intArgs, NULL_CHAR);
                       }

                    while( quantumCyclesRem > 0 && tempPCB->remInstructCycles > 0 
                                                                && !interupt )
                       {
                        // Simulate cpu process running
                        runTime = configDataPtr->procCycleRate;
                        runTimer(runTime);

                        tempPCB->processtimeRemaining -= runTime;
                        if( schedulePreemtive )
                           {
                            quantumCyclesRem--;
                           }
                        tempPCB->remInstructCycles--;
                       }

                    accessTimer(LAP_TIMER, timeStr);
                    if( logToMonitor && tempPCB->remInstructCycles == 0 )
                       {
                        printToMonitor(CPU_END, timeStr, tempPCB->pid,
                            intArgs, NULL_CHAR);
                       }
                    if( logToFile && tempPCB->remInstructCycles == 0 )
                       {
                        currLogLine = printToLogFile(currLogLine, CPU_END,
                            timeStr, tempPCB->pid, intArgs, NULL_CHAR);
                       }
                   }

                if( !isSegFault && tempPCB->remInstructCycles == 0 && !tempPCB->usingIO )
                   {
                    currentInstruction = currentInstruction->nextNode;
                    tempPCB->remInstructCycles = currentInstruction->intArg2;
                    tempPCB->programCounter = currentInstruction->nextNode;
                   }
               }

            accessTimer( LAP_TIMER, timeStr );
            if( logToMonitor && !isSegFault )
               {
                printToMonitor( PROCESS_END, timeStr, tempPCB->pid, 
                                intArgs, NULL_CHAR );
               }
            if( logToFile && !isSegFault )
               {
                 currLogLine = printToLogFile( currLogLine, PROCESS_END, 
                           timeStr, tempPCB->pid, intArgs, NULL_CHAR );
               }

            // Free memory of ended process
            memAllocHead = freeAllocationsByPid( memAllocHead, tempPCB->pid );

            if( logToMonitor )
               {
                printf("After clear process %i success\n", tempPCB->pid);
                printMemStatus( memAllocHead );
               }

            // Set Process state to exited
            tempPCB->processState = EXIT_STATE;
            accessTimer(LAP_TIMER, timeStr);
            if( logToMonitor )
               {
                intArgs[0] = RUNNING_STATE;
                intArgs[1] = EXIT_STATE;
                printToMonitor( STATE_CHANGE, timeStr, tempPCB->pid,
                                intArgs, NULL_CHAR );
               }

            updateProcessPriorities( PCBHead, configDataPtr->cpuSchedCode );
           }

        // Loop back to first PCB
        tempPCB = tempPCB->nextPCB;
       }

    accessTimer( LAP_TIMER, timeStr );
    if( logToMonitor )
       {
        printf( "%s, OS: System stop\n", timeStr );
        printf( "--------------------------------------------------\n" );
        printf( "After clear all process success\n" );
        printf( "No memory configured\n" );
        printf( "--------------------------------------------------\n" );
       }

    if( logToFile )
       {
        sprintf( currLogLine->text, "%s, OS: System stop\n", timeStr );
        currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
        currLogLine = currLogLine->nextLine;
       }

    accessTimer( LAP_TIMER, timeStr );
    printf( "%s, OS: Simulation end\n", timeStr );
    if( logToFile )
       {
        sprintf( currLogLine->text, "%s, OS: Simulation end\n\n", timeStr );
        currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
        currLogLine = currLogLine->nextLine;
        sprintf( currLogLine->text, "End Simulation - Complete\n" );
        currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
        currLogLine = currLogLine->nextLine;
        sprintf( currLogLine->text, "========================= " );
        currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
        currLogLine = currLogLine->nextLine;

        FILE* file = fopen( configDataPtr->logToFileName, WRITE_ONLY_CHAR );
        writeToLogFile( logFileHead, file );
        fclose( file );
        freeLogData( logFileHead );
       }

    accessTimer( STOP_TIMER, timeStr );

    free( timeStr );
    freePCBs( PCBHead );
   }


//returns 0 if correct access, 1 if memory is allocated to different thread and -1 if there is no allocation
bool accessMem( memAllocation *memAllocHead, int base, int limit, int pid )
   {
    memAllocation *tmpMemAlloc = memAllocHead;

    while( tmpMemAlloc != NULL )
       {
        if( base >= tmpMemAlloc->base 
            && (base + limit) <= (tmpMemAlloc->base + tmpMemAlloc->limit) 
            && pid == tmpMemAlloc->pid )
           {
            return true;
           }
        
        tmpMemAlloc = tmpMemAlloc->nextAllocation;
       }

    return false;
   }


// Allocates memory for a process using first fit
memAllocation *allocateMem( memAllocation *memAllocHead, int base, int limit, int pid )
   {
    // Checks if no memory has been allocated so far
    if (memAllocHead == NULL)
       {
        memAllocation* newAlloc = 
              (memAllocation*) malloc( sizeof( memAllocation ) );
        newAlloc->physicalMem = 0;
        newAlloc->base = base;
        newAlloc->limit = limit;
        newAlloc->pid = pid;
        newAlloc->nextAllocation = NULL;

        return newAlloc;
       }

    // Checks if can fit at beginning
    if( memAllocHead->physicalMem > limit ) 
       {
        memAllocation *newHead = (memAllocation*) 
                          malloc( sizeof( memAllocation ) );
        newHead->physicalMem = 0;
        newHead->base = base;
        newHead->limit = limit;
        newHead->pid = pid;
        newHead->nextAllocation = memAllocHead;

        return newHead;
       }

    // Checks if can fit between any 2 allocations in list (First fit)
    memAllocation *firstRef = memAllocHead;
    memAllocation *secRef = memAllocHead->nextAllocation;

    while( secRef != NULL )
       {
        if( limit <= secRef->physicalMem - 
             ( firstRef->physicalMem + firstRef->limit ) )
           {
            memAllocation* newAlloc = 
                  (memAllocation*) malloc( sizeof( memAllocation ) );
            newAlloc->physicalMem = firstRef->physicalMem + firstRef->limit;
            newAlloc->base = base;
            newAlloc->limit = limit;
            newAlloc->pid = pid;
            newAlloc->nextAllocation = secRef;
            firstRef->nextAllocation = newAlloc;

            return memAllocHead;
           }

           firstRef = secRef;
           secRef = secRef->nextAllocation;
       }

    // Checks if there is remaining memory at the end and allocates
    if( limit < MEMORY_LIMIT - ( firstRef->physicalMem + firstRef->limit ) )
       {
        memAllocation* newAlloc = (memAllocation*) malloc(sizeof(memAllocation));
        newAlloc->physicalMem = firstRef->physicalMem + firstRef->limit;
        newAlloc->base = base;
        newAlloc->limit = limit;
        newAlloc->pid = pid;
        newAlloc->nextAllocation = NULL;
        firstRef->nextAllocation = newAlloc;
       }

    return memAllocHead;
   }


// Checks if are PCB are in exit state
bool allProcessesExited( ProcessControlBoard *PCBHead )
   {
    ProcessControlBoard *tempPCB = PCBHead;
    bool allExited = true;

    while( tempPCB != NULL )
       {
        if( tempPCB->processState != EXIT_STATE )
           {
            allExited = false;
           }
        tempPCB = tempPCB->nextPCB;
       }

    return allExited;
   }

// Calculates the remaining time of a process
int calcRemProcTime( ProcessControlBoard *PCB, 
                     int procCycleRate, int ioCycleRate )
   {
    int remainingTime = 0;
    OpCodeType *currentInstruction = PCB->programCounter;

    // Loop until process end
    while( compareString( currentInstruction->command, "app" ) != STR_EQ )
       {
        // check for cpu command
        if( compareString( currentInstruction->command, "cpu" ) == STR_EQ )
           {
            // multiply int arg by procCycleRate
            remainingTime += currentInstruction->intArg2 * procCycleRate;
           }
        // check for dev command
        else if ( compareString( currentInstruction->command, "dev" )
                                                             == STR_EQ )
           {
            // multiply int arg by ioCycleRate
            remainingTime += currentInstruction->intArg2 * ioCycleRate;
           }

        currentInstruction = currentInstruction->nextNode;
       }

    PCB->processtimeRemaining = remainingTime;
    return remainingTime;
   }


void createPCBLinkedList( OpCodeType *metaDataPtr, ProcessControlBoard *PCBHead )
   {
    OpCodeType* currentNode = metaDataPtr;
    ProcessControlBoard *tempPCB = PCBHead;
    ProcessControlBoard *tempPCBPrev;
    int processNum = 0;
    while( currentNode != NULL )
       {
        // Check for the start of a process
        if( ( compareString(currentNode->command, "app") == STR_EQ )
            && ( compareString(currentNode->strArg1, "start") == STR_EQ ) )
           {
            // Append node to linked list
            if( processNum > 0 )
               {
                tempPCB = (ProcessControlBoard*)malloc(sizeof(ProcessControlBoard));
                tempPCBPrev->nextPCB = tempPCB;
               }
            
            tempPCB->pid = processNum;
            processNum++;
            tempPCB->programCounter = currentNode->nextNode;
            tempPCB->processState = NEW_STATE;
            tempPCBPrev = tempPCB;
            tempPCB = tempPCB->nextPCB;
           }

        currentNode = currentNode->nextNode;
       }
   }


OpCodeType *endProcessEarly( OpCodeType *currIntruct)
   {
    while( ( compareString( currIntruct->command, "app" ) != STR_EQ )
        && ( compareString( currIntruct->strArg1, "end" ) != STR_EQ ) )
       {
        currIntruct = currIntruct->nextNode;
       }

    return currIntruct;
   }


// Takes in mem allocation linked list and the pid of memory to free
memAllocation *freeAllocationsByPid( memAllocation *memAllocHead, int pid )
   {
    memAllocation *tmpMemAlloc = memAllocHead;
    memAllocation *newMemAllocHead = memAllocHead;

    if (memAllocHead == NULL)
       {
        return NULL;
       }

    // Free memory at the head if right pid
    while( newMemAllocHead->pid == pid )
       {
        if( newMemAllocHead->nextAllocation == NULL )
           {
            free( newMemAllocHead );
            return NULL;
           }
        newMemAllocHead = newMemAllocHead->nextAllocation;
        free( tmpMemAlloc );
        tmpMemAlloc = newMemAllocHead;
       }

    while( tmpMemAlloc->nextAllocation != NULL )
       {
        if( tmpMemAlloc->nextAllocation->pid == pid )
           {
            memAllocation *memToFree = tmpMemAlloc->nextAllocation;
            // Connects the previous element in the link list 
            // to the one after the freed mem
            // If there is not next element, sets the reference to NULL
            tmpMemAlloc->nextAllocation = ( memToFree->nextAllocation != NULL ) 
                                             ? memToFree->nextAllocation : NULL;
            
            free( memToFree );
           }

        tmpMemAlloc = tmpMemAlloc->nextAllocation;
       }

    return newMemAllocHead;
   }


void freeLogData( LogFileLine *logHeadPtr )
   {
    if( logHeadPtr->nextLine != NULL )
       {
        freeLogData( logHeadPtr->nextLine );
       }
    free( logHeadPtr );
   }


// Recursively frees all PCBs
void freePCBs( ProcessControlBoard *PCB )
   {
    if( PCB->nextPCB != NULL )
       {
        freePCBs( PCB->nextPCB );
       }
    free( PCB );
   }


// Checks to see a portion of memory to be allocated has a section already allocated
bool isAlreadyAllocated(memAllocation* memAllocHead, int base, int limit)
{
    memAllocation* tmpMemAlloc = memAllocHead;

    while (tmpMemAlloc != NULL)
    {
        if ((base + limit) > tmpMemAlloc->base
            && base < (tmpMemAlloc->base + tmpMemAlloc->limit))
        {
            return true;
        }
        tmpMemAlloc = tmpMemAlloc->nextAllocation;
    }
    return false;
}


void printMemStatus( memAllocation *memAllocHead )
   {
    memAllocation *tmpAlloc = memAllocHead;

    // Check for open memory at beginning adress
    if( tmpAlloc == NULL )
       {
        printf( "0 [ Open, P#: x, 0-0 ] %i\n", MEMORY_LIMIT );
       }
    else if( tmpAlloc->physicalMem > 0 )
       {
        printf("0 [ Open, P#: x, 0-0 ] %i", tmpAlloc->physicalMem - 1);
       }

    while( tmpAlloc != NULL )
       {
        // Print used memory
        printf( "%i [ Used, P#: %i, %i-%i ] %i\n", tmpAlloc->physicalMem, 
                                          tmpAlloc->pid, tmpAlloc->base, 
                                   tmpAlloc->base + tmpAlloc->limit - 1, 
                           tmpAlloc->physicalMem + tmpAlloc->limit - 1 );

        // Print remaining open memory at end
        if( tmpAlloc->nextAllocation == NULL )
           { 
            if( (tmpAlloc->physicalMem + tmpAlloc->limit - 1) < MEMORY_LIMIT )
               {
                printf("%i [ Open, P#: x, 0-0 ] %i\n", tmpAlloc->physicalMem 
                                             + tmpAlloc->limit, MEMORY_LIMIT );
               }
           }
        // Print open memory between 2 allocations
        else
           {
            if( ( tmpAlloc->physicalMem + tmpAlloc->limit ) != 
                                     tmpAlloc->nextAllocation->physicalMem )
               {
                printf("%i [ Open, P#: x, 0-0 ] %i\n", tmpAlloc->physicalMem
                    + tmpAlloc->limit, tmpAlloc->nextAllocation->physicalMem - 1);
               }
           }

        tmpAlloc = tmpAlloc->nextAllocation;
       }
    printf("---------------------------");
    printf("-----------------------\n");
   }


LogFileLine *printToLogFile( LogFileLine* currLogLine, int messageCode, char* timeStr,
                     int pid, int intArgs[], char* ioDev )
   {
    char *stateList[] = { "NEW", "READY", "RUNNING", "EXIT" };
    switch( messageCode )
       {
        case STATE_CHANGE:
            sprintf( currLogLine->text, "%s, Process: %i set to %s state from %s"
                                " state\n\n", timeStr, pid, stateList[intArgs[1]],
                                                     stateList[intArgs[0]] );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case PROCESS_SELECTED:
            sprintf( currLogLine->text, "%s, OS: Process %i selected with %i ms"
                                    " remaining\n", timeStr, pid, intArgs[0] );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case PROCESS_END:
            sprintf( currLogLine->text, "%s, OS: Process %i ended\n\n", timeStr,
                                                                      pid );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case CPU_START:
            sprintf( currLogLine->text, "%s, Process: %i, cpu process operation"
                                                 " start\n", timeStr, pid );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case CPU_END:
            sprintf( currLogLine->text, "%s, Process: %i, cpu process operation"
                                                    " end\n", timeStr, pid );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case IO_IN_START:
            sprintf( currLogLine->text, "%s, Process: %i, %s input operation "
                                           "start\n", timeStr, pid, ioDev );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case IO_IN_END:
            sprintf( currLogLine->text, "%s, Process: %i, %s input operation "
                                               "end\n", timeStr, pid, ioDev );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case IO_OUT_START:
            sprintf( currLogLine->text, "%s, Process: %i, %s input operation "
                                            "start\n", timeStr, pid, ioDev );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case IO_OUT_END:
            sprintf( currLogLine->text, "%s, Process: %i, %s input operation "
                                             "end\n", timeStr, pid, ioDev );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case MEM_ALLOC:
            sprintf( currLogLine->text, "%s, Process: %i, mem allocate "
                                           "request (% i, % i)\n",
                                timeStr, pid, intArgs[0], intArgs[1] );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case MEM_ALLOC_SUCC:
            sprintf( currLogLine->text, "%s, Process: %i, successful mem"
                                     " allocate request\n", timeStr, pid );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            
            break;
        case MEM_ALLOC_FAIL:
            sprintf( currLogLine->text, "%s, Process: %i, failed mem"
                                     " allocate request\n\n", timeStr, pid );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case MEM_ACCESS:
            sprintf( currLogLine->text, "%s, Process: %i, mem access "
                                          "request (% i, % i)\n",
                               timeStr, pid, intArgs[0], intArgs[1] );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case MEM_ACCESS_SUCC:
            sprintf( currLogLine->text, "%s, Process: %i, successful mem"
                                      " access request\n", timeStr, pid );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case MEM_ACCESS_FAIL:
            sprintf( currLogLine->text, "%s, Process: %i, failed mem"
                                     " access request\n\n", timeStr, pid );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
            break;
        case OS_SEG_FAULT:
            sprintf( currLogLine->text, "%s, OS : Segmentation fault, Process"
                                      " %i ended\n", timeStr, pid );
            currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
            currLogLine = currLogLine->nextLine;
       }
       return currLogLine;
   }


void printToMonitor( int messageCode, char* timeStr, int pid,
                            int intArgs[], char* ioDev )
   {
    char* stateList[] = { "NEW", "READY", "RUNNING", "EXIT" };
    switch( messageCode )
       {
        case STATE_CHANGE:
            printf("%s, Process: %i set to %s state from %s state\n", timeStr,
                pid, stateList[intArgs[1]], stateList[intArgs[0]]);
            break;
        case PROCESS_SELECTED:
            printf("%s, OS: Process %i selected with %i ms remaining\n",
                timeStr, pid, intArgs[0]);
            break;
        case PROCESS_END:
            printf("%s, OS: Process %i ended\n", timeStr, pid);
            printf("--------------------------------------------------\n");
            break;
        case CPU_START:
            printf("%s, Process: %i, cpu process operation start\n",
                timeStr, pid);
            break;
        case CPU_END:
            printf("%s, Process: %i, cpu process operation end\n",
                timeStr, pid);
            break;
        case IO_IN_START:
            printf("%s, Process: %i, %s input operation start\n", timeStr,
                pid, ioDev);
            break;
        case IO_IN_END:
            printf("%s, Process: %i, %s input operation end\n", timeStr,
                pid, ioDev);
            break;
        case IO_OUT_START:
            printf("%s, Process: %i, %s input operation start\n", timeStr,
                pid, ioDev);
            break;
        case IO_OUT_END:
            printf("%s, Process: %i, %s input operation end\n", timeStr,
                pid, ioDev);
            break;
        case MEM_ALLOC:
            printf( "%s, Process: %i, mem allocate request(% i, % i)\n",
                                timeStr, pid, intArgs[0], intArgs[1] );
            break;
        case MEM_ALLOC_SUCC:
            printf( "---------------------------" 
                    "-----------------------\n" );
            printf( "After allocation success\n" );
            break;
        case MEM_ALLOC_FAIL:
            printf( "---------------------------" 
                    "-----------------------\n" 
                    "After allocation failure\n" );
            break;
        case MEM_ACCESS:
            printf( "%s, Process: %i, mem allocate request(% i, % i)\n",
                           timeStr, pid, intArgs[0], intArgs[1] );
            break;
        case MEM_ACCESS_SUCC:
            printf( "---------------------------" 
                    "-----------------------\n" 
                    "After access success\n" );
            break;
        case MEM_ACCESS_FAIL:
            printf( "---------------------------"
                    "-----------------------\n" 
                    "After access failure\n" );
            break;
        case OS_SEG_FAULT:
            printf( "%s, OS : Segmentation fault, Process %i"
                       " ended\n", timeStr, pid);
            printf( "---------------------------"
                    "-----------------------\n" );
            break;
       }
   }

void *runTimerThread( void* milliSeconds )
   {
    int mSecs = *((int*)milliSeconds);
    runTimer(mSecs);
    return NULL;
   }

void *runTimerPreemptive( void* args )
   {
    ThreadArgs *ta = (ThreadArgs *)args;
    int intArg = 0;
    int *intArgs = &intArg;
    runTimer(ta->milliSeconds);
    if( ta->isFCFSP )
       {
        ta->PCB->schedPriority = ta->numProcesses - 1;
       }
    ta->PCB->processtimeRemaining -= ta->milliSeconds;

    accessTimer( LAP_TIMER, ta->timeStr );
    if( ta->logToMonitor )
       {
        printf("logging to monitor\n");
        printToMonitor( ta->ioType[1], ta->timeStr, ta->PCB->pid,
            intArgs, ta->PCB->programCounter->strArg1 );
       }
    if( ta->logToFile )
       {
        ta->currLine = printToLogFile( ta->currLine, ta->ioType[1],
            ta->timeStr, ta->PCB->pid, intArgs,
            ta->PCB->programCounter->strArg1 );
       }

    printf("interupting\n");
    *ta->interupt = true;
    ta->PCB->usingIO = false;
    *ta->ioInUse = false;
    printf("Thread done\n");

    return NULL;
   }

// Iterates through PCB linked list assigning priotities
void setProcessPriorities( ProcessControlBoard *PCBHead, int scheduleType )
   {
    ProcessControlBoard *tempPCB = PCBHead;
    int index, numPCBs;

    switch( scheduleType )
       {
        case CPU_SCHED_FCFS_N_CODE:
        case CPU_SCHED_FCFS_P_CODE:
        case CPU_SCHED_RR_P_CODE:
            index = 0;
            while( tempPCB != NULL )
               {
                tempPCB->schedPriority = index;
                index++;
                tempPCB = tempPCB->nextPCB;
               }
            break;

        case CPU_SCHED_SJF_N_CODE:
        case CPU_SCHED_SRTF_P_CODE:
            numPCBs = 0;
            // Initialize all PCB scheduling priorities to -1 (unassigned)
            while( tempPCB != NULL )
               {
                tempPCB->schedPriority = -1;
                numPCBs++;
                tempPCB = tempPCB->nextPCB;
               }

            for( index = 0; index < numPCBs; index++ )
               {
                int lowestRemTime = 0;
                tempPCB = PCBHead;

                // Search for lowest remaining time
                while( tempPCB != NULL )
                   {
                    if (lowestRemTime == 0 && tempPCB->schedPriority == -1)
                       {
                        lowestRemTime = tempPCB->processtimeRemaining;
                       }
                    else if( tempPCB->processtimeRemaining < lowestRemTime 
                                    && tempPCB->schedPriority == -1 )
                       {
                        lowestRemTime = tempPCB->processtimeRemaining;
                       }
                    tempPCB = tempPCB->nextPCB;
                   }

                // Shedule the next PCB with lowest remaining time 
                tempPCB = PCBHead;
                while( tempPCB != NULL )
                   {
                    if( tempPCB->processtimeRemaining == lowestRemTime ) 
                       {
                        tempPCB->schedPriority = index;
                       }
                    tempPCB = tempPCB->nextPCB;
                   }
               }

            tempPCB = PCBHead;
            while (tempPCB != NULL)
            {
                tempPCB = tempPCB->nextPCB;
            }
            break;
       }
   }


// Decrements the priority of each process
void updateProcessPriorities( ProcessControlBoard *PCBHead, int schedType )
   {
    ProcessControlBoard *tempPCB = PCBHead;
    if( schedType == CPU_SCHED_RR_P_CODE ) 
       {
        ProcessControlBoard* highPriorPCB;
        ProcessControlBoard* startPCB = tempPCB;
        int numProcesses = 0;
        
        while( tempPCB != NULL )
           {
            if( tempPCB->schedPriority == 0 )
               {
                highPriorPCB = tempPCB;
               }
            
            tempPCB = tempPCB->nextPCB;
            numProcesses++;
           }

        tempPCB = startPCB;
        while( tempPCB != NULL )
           {
            if( tempPCB == highPriorPCB )
               {
                tempPCB->schedPriority = numProcesses - 1;
               }
            else
               {
                tempPCB->schedPriority--;
               }
            tempPCB = tempPCB->nextPCB;
           }
       }
    else if(schedType == CPU_SCHED_SRTF_P_CODE)
       {
        setProcessPriorities(PCBHead, schedType);
       }
    // FCFS-P gets sheduled when an interupt occurs, otherwise acts like non-preemtive
    else 
       {
        while( tempPCB != NULL )
           {
            tempPCB->schedPriority--;
            tempPCB = tempPCB->nextPCB;
           }
       }
   }


// TODO: Change log datatype from string to linked-list
LogFileLine *writeLogFileHeader( LogFileLine *logFileHead, 
                                ConfigDataType *configDataPtr )
   {
    LogFileLine *currLogLine = logFileHead;
    char *cpuSchedCodes[] = { "SJF-N", "SRTF-P", "FCFS-P", "RR-P", "FCFS-N" };
    sprintf( currLogLine->text, "\n=============================="
                                                   "====================\n" );
    currLogLine->nextLine = (LogFileLine*) malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;

    sprintf( currLogLine->text, "Simulator Log File Header\n\n" );
    currLogLine->nextLine = (LogFileLine*) malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;

    sprintf( currLogLine->text, "File Name                       : %s\n", 
                                           configDataPtr->metaDataFileName );
    currLogLine->nextLine = (LogFileLine*) malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;

    sprintf( currLogLine->text, "CPU Scheduling                  : %s\n",
                               cpuSchedCodes[ configDataPtr->cpuSchedCode ] );
    currLogLine->nextLine = (LogFileLine*) malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;

    sprintf( currLogLine->text, "Quantum Cycles                  : %i\n",
                                           configDataPtr->quantumCycles );
    currLogLine->nextLine = (LogFileLine*) malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;

    sprintf(currLogLine->text, "Memory Available (KB)           : %i\n", 
                                            configDataPtr->memAvailable );
    currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;
    
    sprintf( currLogLine->text, "Processor Cycle Rate (ms/cycle) : %i\n", 
                                           configDataPtr->procCycleRate );
    currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;
    
    sprintf(currLogLine->text, "I/O Cycle Rate (ms/cycle)       : %i\n", 
                                             configDataPtr->ioCycleRate );
    currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;
    
    sprintf(currLogLine->text, "\n================\n" );
    currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;

    sprintf(currLogLine->text, "Begin Simulation\n\n" );
    currLogLine->nextLine = (LogFileLine*)malloc(sizeof(LogFileLine));
    currLogLine = currLogLine->nextLine;

    return currLogLine;
   }


void writeToLogFile( LogFileLine* logHeadPtr, FILE *logFileOut )
   {
    LogFileLine *tmpPtr = logHeadPtr;

    while( tmpPtr != NULL )
       {
        fprintf( logFileOut, tmpPtr->text );
        tmpPtr = tmpPtr->nextLine;
       }
   }