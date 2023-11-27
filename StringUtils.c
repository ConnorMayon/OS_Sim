#include "StringUtils.h"

int compareString( const char *oneStr, const char *otherStr )
   {
    int diff, index = 0;
    
    while( oneStr[ index ] != NULL_CHAR 
           && otherStr[ index ] != NULL_CHAR 
           && index < MAX_STR_LEN )
       {
        diff = oneStr[ index ] - otherStr[ index ];

        if( diff != 0 )
           {
            return diff;
           }

        index++;
       }
    
    return getStringLength( oneStr ) - getStringLength( otherStr );
   }

void concatenateString( char *destStr, const char *sourceStr )
   {
    int destIndex = getStringLength( destStr );
    int sourceStrLen = getStringLength( sourceStr );
    char *tempStr;
    int sourceIndex = 0;

    tempStr = (char *)malloc( sizeof( sourceStrLen + 1 ) );
    copyString( tempStr, sourceStr );

    while( tempStr[ sourceIndex ] != NULL_CHAR && destIndex < MAX_STR_LEN )
       {
        destStr[ destIndex ] = tempStr[ sourceIndex ];
        destIndex++;
        sourceIndex++;
        destStr[ destIndex ] = NULL_CHAR;
       }

    free( tempStr );
   }

void copyString( char *destStr, const char *sourceStr )
   {
    int index = 0;

    if( destStr != sourceStr )
       {
        while( sourceStr[ index ] != NULL_CHAR && index < MAX_STR_LEN )
           {
            destStr[ index ] = sourceStr[ index ];
            index++;
            destStr[ index ] = NULL_CHAR;
           }
       }
   }

int findSubString( const char *testStr, const char *searchSubStr )
   {
    int testStrLen = getStringLength( testStr );
    int masterIndex = 0;
    int searchIndex, internalIndex;

    while( masterIndex < testStrLen )
       {
        internalIndex = masterIndex;
        searchIndex = 0;
 
        while( internalIndex <= testStrLen 
               && testStr[ internalIndex ] == searchSubStr[ searchIndex ] )
           {
            internalIndex++;
            searchIndex++;

            if( searchSubStr[ searchIndex ] == NULL_CHAR )
               {
                return masterIndex;
               }
           }

        masterIndex++;
       }

    return SUBSTRING_NOT_FOUND;
   }

bool getStringConstrained(
                          FILE *inStream,
                          bool clearLeadingNonPrintable,
                          bool clearLeadingSpace,
                          bool stopAtNonPrintable,
                          char delimiter,
                          char *capturedString
                         )
   {
    int intChar = EOF, index = 0;
    capturedString[ index ] = NULL_CHAR;
    intChar = fgetc( inStream );

    while( ( intChar != EOF ) 
             && ( ( clearLeadingNonPrintable && intChar < (int)SPACE )
             || ( clearLeadingSpace && intChar == (int)SPACE ) )
         )
       {
        intChar = fgetc( inStream );
       }

    if( intChar == EOF )
       {
        return false;
       }

    while( ( intChar != EOF && index < MAX_STR_LEN - 1 )
           && ( ( stopAtNonPrintable && intChar >= (int)SPACE )
             || ( !stopAtNonPrintable ) )
           && ( intChar != (char)delimiter )
         )
       {
        capturedString[ index ] = (char)intChar;
        index++;
        intChar = fgetc( inStream );
       }

    return true;
   }

int getStringLength( const char *testStr )
   {
    int index = 0;
    while( index < STD_STR_LEN && testStr[ index ] != NULL_CHAR )
       {
        index++;
       }
    return index;
   }

bool getStringToDelimiter(
                          FILE *inStream,
                          char delimiter,
                          char *capturedString
                         )
   {
    return getStringConstrained( inStream, true, true, true, 
                                 delimiter, capturedString );
   }

bool getStringToLineEnd(
                        FILE *inStream,
                        char *capturedString
                       )
   {
    return getStringConstrained( inStream, true, true, true,
                                 NON_PRINTABLE_CHAR, capturedString );
   }

void getSubString( char *destStr, const char *sourceStr, 
                                    int startIndex, int endIndex )
   {
    int sourceStrLen = getStringLength( sourceStr );
    int destIndex = 0;
    int sourceIndex = startIndex;
    char *tempStr;

    if( startIndex >= 0 && startIndex <= endIndex && endIndex < sourceStrLen )
       {
        tempStr = (char *)malloc( sourceStrLen + 1 );
        copyString( tempStr, sourceStr );

        while( sourceIndex <= endIndex )
           {
            destStr[ destIndex ] = tempStr[ sourceIndex ];
            destIndex++;
            sourceIndex++;
            destStr[ destIndex ] = NULL_CHAR;
           }

        free( tempStr );
       }
   }

void setStrToLowerCase( char *destStr, const char *sourceStr )
   {
    int sourceStrLen = getStringLength( sourceStr );
    char *tempStr;
    int index = 0;
    
    tempStr = (char *)malloc( sizeof( sourceStrLen + 1 ) );
    copyString( tempStr, sourceStr );

    while( tempStr[ index ] != NULL_CHAR && index < MAX_STR_LEN )
       {
        destStr[ index ] = toLowerCase( tempStr[ index ] );
        index++;
        destStr[ index ] = NULL_CHAR;
       }

    free( tempStr );
   }

char toLowerCase( char testChar )
   {
    if( testChar >= 'A' && testChar <= 'Z' )
       {
        return testChar - 'A' + 'a';
       }

    return testChar;
   }
