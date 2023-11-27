#include "simulator.h"

// Main function to run simulator
void runSim( ConfigDataType *configDataPtr, OpCodeType *metaDataPtr )
   {
    // Initialize variables
    int numProcesses, numInstructs, schedType, index, logSize, timeRem;
    bool logToMonitor = true, logToFile = true;
    char *timeStr, *logStrOut;

    numProcesses = getNumProcesses( metaDataPtr );
    numInstructs = countNumInstructs( metaDataPtr );

    schedType = configDataPtr->cpuSchedCode;
// TODO: Turn PCBArray into a linked list
    ProcessControlBoard PCBarray[ numProcesses ];
    int processStartArray[ numProcesses ];
    timeStr = (char*) malloc( sizeof(char) * MIN_STR_LEN );
    logSize = (numProcesses * 50 * 5) + (numInstructs * 50 * 2) + (20 * 50);
    logStrOut = (char*) malloc( sizeof(char) * logSize );

    if( configDataPtr->logToCode == LOGTO_FILE_CODE )
       {
        logToMonitor = false;
       }
    else if( configDataPtr->logToCode == LOGTO_FILE_CODE )
       {
        logToFile = false;
       }

// TODO: Logging to file causes seg fault, change data type
logToFile = false;
    
    // Get process start indices, exit if error
    if( findProcessStarts( metaDataPtr, numProcesses, processStartArray ) != 0 )
       {
        fprintf( stderr, "Number of processes does not match meta-data" );
        exit( 1 );
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
    if (logToMonitor)
       {
        printf( "%s, OS: Simulator start\n", timeStr );
       }
    
    // Update PCB id, state, and program counter
    fillPCB( PCBarray, metaDataPtr, processStartArray, numProcesses, timeStr );

    // Set all states to ready
    for( index = 0; index < numProcesses; index++ )
       {
        PCBarray[index].processState = READY_STATE;
        accessTimer( LAP_TIMER, timeStr );
        if( logToMonitor )
            {
             timeRem = 0;
             printToMonitor( STATE_CHANGE, timeStr, index, 
                                     NEW_STATE, READY_STATE, timeRem );
            }
       }

    printf("\n");

    // Check scheduling type
    switch( schedType )
       {
        // Call corresponding scheduling function
        case CPU_SCHED_FCFS_N_CODE:
            schedFCFSN( PCBarray, numProcesses, configDataPtr, 
                        timeStr, logStrOut, logToMonitor, logToFile );
            break;
        default:
            printf("Scheduling currently not supported. Switching to FCFSN\n");
            schedFCFSN( PCBarray, numProcesses, configDataPtr, 
                        timeStr, logStrOut, logToMonitor, logToFile );
            break;
       }

    accessTimer( LAP_TIMER, timeStr );
    if( logToMonitor )
       {
        printf( "%s, OS: System stop\n", timeStr );
        printf( "--------------------------------------------------\n" );
       }

    // TODO: clear memory of processes

    accessTimer( LAP_TIMER, timeStr );
    if( logToMonitor )
       {
        printf( "%s, OS: Simulation end\n", timeStr );
       }

    accessTimer( STOP_TIMER, timeStr );
   }


// Calculates the remaining time of a process
int calcRemProcTime( ProcessControlBoard PCB, 
                     int procCycleRate, int ioCycleRate )
   {
    int remainingTime = 0;
    OpCodeType *currentInstruction = PCB.programCounter;

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
        else
           {
            // multiply int arg by ioCycleRate
            remainingTime += currentInstruction->intArg2 * ioCycleRate;
           }

        currentInstruction = currentInstruction->nextNode;
       }

    return remainingTime;
   }


// Counts the number of cpu and io instructions in meta-data
int countNumInstructs( OpCodeType *metaDataPtr )
   {
    int numInstructs = 0;
    OpCodeType *currentInstruction = metaDataPtr;

    // Loop until process end
    while( currentInstruction != NULL )
       {
        if( compareString( currentInstruction->command, "cpu" ) == STR_EQ 
            || compareString( currentInstruction->command, "dev" ) == STR_EQ )
           {
            numInstructs++;
           }
        currentInstruction = currentInstruction->nextNode;
       }

    return numInstructs;
   }


// Fills PCB struct with correct pid and sets program counter from meta-data 
// Input: PCBArray to store data, meta-data linked list pointer
void fillPCB( ProcessControlBoard *PCBarray, OpCodeType *metaDataPtr, 
              int *processStartArray, int numProcesses, char *timeStr )
   {
    int count, processStartIndex, index = 0;
    OpCodeType *currentNode = metaDataPtr;

    // Loop through process start array
    for( count = 0; count < numProcesses; count++ )
       {
        processStartIndex = processStartArray[ count ];

        // Set current node to index of start of each process
        while( index < processStartIndex )
           {
            currentNode = currentNode->nextNode;
            index++;
           }
        
        // Update PCB
        PCBarray[ count ].pid = count;
        PCBarray[ count ].programCounter = currentNode->nextNode;
        PCBarray[ count ].processState = NEW_STATE;
       }        
   }


// Finds the index to the start of each process in meta-data linked list
// Input: Pointer to meta-data linked list
// Output: 0 if no error
int findProcessStarts( OpCodeType *metaDataPtr, int numProcesses, int *indexArr )
   {
    int index = 0, count = 0;
    OpCodeType *currentNode = metaDataPtr;
    
    while( count < numProcesses )
       {
        // Check for the start of a process
        if( (compareString( currentNode->command, "app" ) == STR_EQ) 
             && (compareString( currentNode->strArg1, "start" ) == STR_EQ) )
           {
            // Apend index to array
            indexArr[ count ] = index;
            count++;
           }

        if( currentNode->nextNode == NULL )
           {
            return 1;
           }

        currentNode = currentNode->nextNode;
        index++;
       }

    return 0;
   }


// Finds the number of processes in meta-data
// Input: Pointer to meta-data
// Output: Pid of the last node
int getNumProcesses( OpCodeType *metaDataPtr )
   {
    int numProcesses = 0;
    OpCodeType *currentNode = metaDataPtr;
    while( currentNode->nextNode != NULL )
       {
        if(compareString( currentNode->strArg1, "start" ) == STR_EQ)
           {
            numProcesses++;
           }
        currentNode = currentNode->nextNode;
       }

    // subtract 1 due to sys start
    return numProcesses - 1;
   }


void schedFCFSN( ProcessControlBoard *PCBarray, int numProcesses, 
                 ConfigDataType *configDataPtr, char *timeStr, 
                 char *logStr, bool logToMonitor, bool logToFile )
   {
    int index, runTime;
    int *runTimePtr;
    pthread_t threadNum;

    // Loop through processes
    for( index = 0; index < numProcesses; index++ )
       {
        ProcessControlBoard currentPCB = PCBarray[ index ];
        OpCodeType *currentInstruction = currentPCB.programCounter;
        int remainingTime = calcRemProcTime( currentPCB, 
                                             configDataPtr->procCycleRate, 
                                             configDataPtr->ioCycleRate );

        accessTimer( LAP_TIMER, timeStr );
        if( logToMonitor )
           {
        printToMonitor( PROCESS_SELECTED, timeStr, currentPCB.pid, 
                             RUNNING_STATE, RUNNING_STATE, remainingTime );
           }
        // Set Process state to running
        PCBarray[index].processState = RUNNING_STATE;
        accessTimer( LAP_TIMER, timeStr );
        if( logToMonitor )
            {
             runTime = 0;
             printToMonitor( STATE_CHANGE, timeStr, index, 
                                     READY_STATE, RUNNING_STATE, runTime );
            }
        printf("\n");

        // while program counter command is not end
        while( (compareString( currentInstruction->command, "app" ) != STR_EQ) 
           && (compareString( currentInstruction->strArg1, "end" ) != STR_EQ) )
           {
            // if cpu process, wait
            if( compareString( currentInstruction->command, "cpu" ) == STR_EQ )
               {
                accessTimer( LAP_TIMER, timeStr );
                printf( "%s, Process: %i, cpu process operation start\n", 
                                                   timeStr, currentPCB.pid );

                // Simulate cpu process running
                runTime = currentInstruction->intArg2 
                                * configDataPtr->procCycleRate;
                runTimer( runTime );

                accessTimer( LAP_TIMER, timeStr );
                printf( "%s, Process: %i, cpu process operation end\n", 
                                                    timeStr, currentPCB.pid );
               }

            // if io
            else 
               {
                char *ioStr[] = {"input", "output"};
                int ioIndex;

                // check for input or output
                if (compareString( currentInstruction->inOutArg, "in" ) == STR_EQ)
                   {
                    ioIndex = 0;
                   }
                else
                   {
                    ioIndex = 1;
                   }

                accessTimer( LAP_TIMER, timeStr );
                printf( "%s, Process: %i, %s %s operation start\n", timeStr, 
                                currentPCB.pid, currentInstruction->strArg1, 
                                                              ioStr[ioIndex] );

                runTime = currentInstruction->intArg2 
                                       * configDataPtr->ioCycleRate;
                runTimePtr = &runTime;
                
                // Simulate io running
                pthread_create( &threadNum, NULL, runTimerThread, 
                                              (void *)runTimePtr );
                pthread_join( threadNum, NULL );

                accessTimer( LAP_TIMER, timeStr );
                printf( "%s, Process: %i, %s %s operation end\n", timeStr, 
                              currentPCB.pid, currentInstruction->strArg1, 
                                                           ioStr[ioIndex] );
               }

            currentInstruction = currentInstruction->nextNode;          
           }
          
        accessTimer( LAP_TIMER, timeStr );
        if( logToMonitor )
           {
            printf( "\n%s, OS: Process %i ended\n", timeStr, currentPCB.pid );
            printf( "--------------------------------------------------\n" );
           }
        if( logToFile )
           {
            concatenateString( logStr, "Process ended" );
           }

        // Set Process state to exited
        PCBarray[index].processState = EXIT_STATE;
        accessTimer( LAP_TIMER, timeStr );
        if( logToMonitor )
            {
             runTime = 0;
             printToMonitor( STATE_CHANGE, timeStr, index, 
                                     RUNNING_STATE, EXIT_STATE, runTime );
            }
       }
   }

// TODO: Change log datatype from string to linked-list
void writeLogFileHeader( ConfigDataType *configDataPtr, char *logStr )
   {
/*
    char *cpuSchedCodes[] = { "SJF-N", "SRTF-P", "FCFS-P", "RR-P", "FCFS-N" };
    concatenateString( logStr, "\n========================" );
    concatenateString( logStr, "==========================" );
    concatenateString( logStr, "\nSimulator Log File Header\n\n" );
    concatenateString( logStr, "File Name                       : " );
    concatenateString( logStr, configDataPtr->metaDataFileName );
    concatenateString( logStr, "\nCPU Scheduling                  : " );
    concatenateString( logStr, (char *) cpuSchedCodes[configDataPtr->cpuSchedCode] );
    concatenateString( logStr, "\nQuantum Cycles                  : " );
    concatenateString( logStr, (char *) configDataPtr->quantumCycles );
    concatenateString( logStr, "\nMemory Available (KB)           : " );
    concatenateString( logStr, (char *) configDataPtr->memAvailable );
    concatenateString( logStr, "\nProcessor Cycle Rate (ms/cycle) : " );
    concatenateString( logStr, (char *) configDataPtr->procCycleRate );
    concatenateString( logStr, "\nI/O Cycle Rate (ms/cycle)       : " );
    concatenateString( logStr, (char *) configDataPtr->ioCycleRate );
    concatenateString( logStr, "\n\n================\n" );
    concatenateString( logStr, "Begin Simulation\n\n" );
*/
   }

// TODO: Bug in CPU start and end codes causing messages to print before timer
void printToMonitor( int messageCode, char *timeStr, int processNum, 
                                     int prevState, int currState, int timeRem )
   {
    char *stateList[] = { "NEW", "READY", "RUNNING", "EXIT" };
    switch( messageCode )
       {
        case STATE_CHANGE:
            printf( "%s, Process: %i set to %s state from %s state\n", timeStr, 
                      processNum, stateList[ currState ], stateList[ prevState ] );
        case PROCESS_SELECTED:
            printf( "%s, OS: Process %i selected with %i ms remaining\n", 
                           timeStr, processNum, timeRem  );

       }     
   }
