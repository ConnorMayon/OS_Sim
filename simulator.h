// Preprocessor directive
#ifndef SIMULATOR_H
#define SIMULATOR_H

// header files
#include <pthread.h>
#include "datatypes.h"
#include "StringUtils.h"
#include "simtimer.h"
#include "configops.h"

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
               NO_TIME_REM = -1 } MessageCode;

typedef struct ProcessControlBoard
   {
    int pid;
    int processState;
    OpCodeType *programCounter;
    int schedPriority;
    int timeUsed;
    int processtimeRemaining;
    int IOTimeRemaining;
   } ProcessControlBoard;

int calcRemProcTime( ProcessControlBoard PCB, int procCycleRate, int ioCycleRate );
void changeProcessState( ProcessControlBoard PCB, int state, char *timeStr, 
                         char *logStr, bool logToMonitor, bool logToFile );
int countNumInstructs( OpCodeType *metaDataPtr );
int findProcessStarts( OpCodeType *metaDataPtr, int numProcesses, int *indexArr );
void fillPCB( ProcessControlBoard *PCBarray, OpCodeType *metaDataPtr, 
              int *processStartArray, int numProcesses, char *timeStr );
int getNumProcesses( OpCodeType *metaDataPtr );
void printToMonitor( int messageCode, char *timeStr, int processNum, 
                          int prevState, int currState, int timeRem );
void runSim( ConfigDataType *configDataPtr, OpCodeType *metaDataPtr );
void schedFCFSN( ProcessControlBoard *PCBarray, int numProcesses, 
                 ConfigDataType *configDataPtr, char *timeStr, char *logStr, 
                 bool logToMonitor, bool logToFile );
void writeLogFileHeader( ConfigDataType *configDataPtr, char *logStr );

#endif // SIMULATOR_H