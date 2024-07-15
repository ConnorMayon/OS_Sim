// Preprocessor directive
#ifndef SIMULATOR_H
#define SIMULATOR_H

// header files
#include <pthread.h>
#include "datatypes.h"
#include "StringUtils.h"
#include "simtimer.h"
#include "configops.h"

#define MEMORY_LIMIT 11099

// Five state data structure for processes
typedef enum { NEW_STATE,
               READY_STATE,
               RUNNING_STATE,
               EXIT_STATE } ProcessState;

typedef enum { STATE_CHANGE,
               PROCESS_SELECTED,
               PROCESS_END,
               CPU_START,
               CPU_END,
               IO_IN_START,
               IO_IN_END,
               IO_OUT_START,
               IO_OUT_END,
               MEM_ALLOC,
               MEM_ALLOC_SUCC,
               MEM_ALLOC_FAIL,
               MEM_ACCESS,
               MEM_ACCESS_SUCC,
               MEM_ACCESS_FAIL,
               OS_SEG_FAULT,
               OS_SYS_STOP,
               UNUSED_ARG = -1 } MessageCode;

typedef struct LogFileLine
{
    char text[ MAX_STR_LEN ];
    struct LogFileLine *nextLine;
} LogFileLine;

typedef struct memAllocation
{
    int base;
    int limit;
    int physicalMem;
    int pid;
    struct memAllocation *nextAllocation;
} memAllocation;

typedef struct ProcessControlBoard
   {
    int pid;
    int processState;
    int schedPriority;
    int timeUsed;
    int processtimeRemaining;
    int remInstructCycles;
    bool usingIO;
    OpCodeType *programCounter;
    memAllocation processAllocations;
    struct ProcessControlBoard *nextPCB;
   } ProcessControlBoard;

typedef struct ThreadArgs
{
    int milliSeconds;
    int numProcesses;
    bool isFCFSP;
    bool logToMonitor;
    bool logToFile;
    bool *ioInUse;
    bool *interupt;
    int *ioType;
    char *timeStr;
    LogFileLine *currLine;
    ProcessControlBoard *PCB;
} ThreadArgs;


bool accessMem( memAllocation *memAllocHead, int base, int limit, int pid );
memAllocation *allocateMem( memAllocation *memAllocHead, int base, 
                            int limit, int pid );
bool allProcessesExited( ProcessControlBoard *PCBHead );
int calcRemProcTime( ProcessControlBoard *PCB, int procCycleRate, 
                                                   int ioCycleRate );
void createPCBLinkedList( OpCodeType *metaDataPtr, 
                                     ProcessControlBoard *PCBHead );
OpCodeType *endProcessEarly( OpCodeType *currIntruct );
memAllocation *freeAllocationsByPid( memAllocation *memAllocHead, int pid );
void freeLogData( LogFileLine* logHeadPtr );
void freePCBs( ProcessControlBoard *PCB );
bool isAlreadyAllocated( memAllocation* memAllocHead, int base, int limit );
void printMemStatus( memAllocation *memAllocHead );
LogFileLine *printToLogFile( LogFileLine* currLogLine, int messageCode, char* timeStr,
                     int pid, int intArgs[], char* ioDev );
void printToMonitor( int messageCode, char *timeStr, int pid, 
                       int intArgs[], char* ioDev);
void runSim( ConfigDataType *configDataPtr, OpCodeType *metaDataPtr );

void *runTimerThread( void *milliSeconds );
void *runTimerPreemptive( void *args );
void setProcessPriorities( ProcessControlBoard *PCBHead, int scheduleType );
void updateProcessPriorities( ProcessControlBoard *PCBHead, int schedType );
LogFileLine *writeLogFileHeader( LogFileLine *logFileHead,
                                          ConfigDataType* configDataPtr );
void writeToLogFile( LogFileLine* logHeadPtr, FILE* logFileOut );

#endif // SIMULATOR_H