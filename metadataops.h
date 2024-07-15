/*
 * Author: Michael Leveringon
*/

// Preprocessor directive
#ifndef METADATAOPS_H
#define METADATAOPS_H

// header files
#include "datatypes.h"
#include "StringUtils.h"

typedef enum {
    BAD_ARG_VAL = -1,
    NO_ACCESS_ERR,
    MD_FILE_ACCESS_ERR,
    MD_CORRUPT_DESCRIPTOR_ERR,
    OPCMD_ACCESS_ERR,
    CORRUPT_OPCMD_ERR,
    CORRUPT_OPCMD_ARG_ERR,
    UNBALANCED_START_END_ERR,
    COMPLETE_OPCMD_FOUND_MSG,
    LAST_OPCMD_FOUND_MSG } OpCodeMessages;

// prototypes
OpCodeType *addNode( OpCodeType *localPtr, OpCodeType *newNode );
OpCodeType *clearMetaDataList( OpCodeType *localPtr );
void displayMetaData( OpCodeType *localPtr );
int getCommand( char *cmd, char *inputStr, int index );
int getOpCommand( FILE *filePtr, OpCodeType *inData );
bool getMetaData( char *fileName, OpCodeType **opCodeDataHead, char *endStateMsg );
int getNumberArg( int *number, char *inputStr, int index );
int getStringArg( char *strArg, char *inputStr, int index );
bool isDigit( char testChar );
int updateStartCount( int count, char *opString );
int updateEndCount( int count, char *opString );
bool verifyValidCommand( char *testCmd );
bool verifyFirstStringArg( char *strArg );

#endif // METADATAOPS_H
