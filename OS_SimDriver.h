/*
 * Author: Connor Mayon
 */

// Preprocessor directive
#ifndef OS_SIM_DRIVER_H
#define OS_SIM_DRIVER_H

// header files
#include <pthread.h>
#include "configops.h"
#include "metadataops.h"
#include "simulator.h"
#include "simtimer.h"

// Program constants
typedef enum { MIN_NUM_ARGS = 3, LAST_FOUR_LETTERS = 4 } PRGRK_CONSTANTS;

// Command line struct for storing command line switch settings
typedef struct CmdLineDataStruct
   {
    bool programRunFlag;
    bool configDisplayFlag;
    bool mdDisplayFlag;
    bool runSimFlag;

    char fileName[ STD_STR_LEN ];
   } CmdLineData;

// Function prototypes

void clearCmdLineStruct( CmdLineData *clDataPtr );
bool processCmdLine( int numArgs, char **strVector, CmdLineData *clDataPtr );
void showCommandLineFormat();

#endif // OS_SIM_DRIVER_H
