#ifndef DATATYPES_H
#define DATATYPES_H

// header files
#include <stdio.h>
#include "StandardConstants.h"

// GLOBAL CONSTANTS - may be used in other files

typedef enum { CMD_STR_LEN = 5,
               IO_ARG_STR_LEN = 5,
               STR_ARG_LEN = 15 } OpCodeArrayCapacity;

typedef struct ConfigDataType
   {
    double version;
    char metaDataFileName[ 100 ];
    int quantumCycles;
    bool memDisplay;
    int memAvailable;
    int procCycleRate;
    int ioCycleRate;
    char logToFileName[ 100 ];
    int cpuSchedCode;
    int logToCode;
   } ConfigDataType;

typedef struct OpCodeType
   {
    int pid;
    char command[ 5 ];
    char inOutArg[ 5 ];
    char strArg1[ 15 ];
    int intArg2;
    int intArg3;

    double opEndTime;
    struct OpCodeType *nextNode;
   } OpCodeType;

#endif // DATATYPES_H
