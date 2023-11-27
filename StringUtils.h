// Preprocessor directive
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

// header files
#include <stdio.h>
#include <stdlib.h>
#include "StandardConstants.h"

typedef enum { NO_ERR,
               INCOMPLETE_FILE_ERR,
               INPUT_BUFFER_OVERRUN_ERR } StringManipCode;

// prototypes
int compareString( const char *oneStr, const char *otherStr );
void concatenateString( char *destStr, const char *sourceStr );
void copyString( char *destStr, const char *sourceStr );
int findSubString( const char *testStr, const char *searchSubStr );
bool getStringConstrained(
                          FILE *inStream,
                          bool clearLeadingNonPrintable,
                          bool clearLeadingSpace,
                          bool stopAtNonPrintable,
                          char delimiter,
                          char *capturedString
                         );
int getStringLength( const char *testStr );
bool getStringToDelimiter(
                          FILE *inStream,
                          char delimiter,
                          char *capturedString
                         );
bool getStringToLineEnd(
                        FILE *inStream,
                        char *capturedString
                       );
void getSubString( char *destStr, const char *sourceStr, int startIndex, int endIndex );
void setStrToLowerCase( char *destStr, const char *sourceStr );
char toLowerCase( char testChar );

#endif // STRING_UTILS_H
