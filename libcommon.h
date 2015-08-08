//  lib_parser - A parser tool.
//
//  Release  Note:
//    lib_parser-1.0.0  :
//      Author : Albert Huang
//      Release Date: 2012-11-28
//      Enhancements and Improvements:
//        #  Implement Parser_Sprintf()
//        #  Implement Parser_Skip()
//      Bugfix:
//      Maintenance:

#ifndef  __ALRIGHT_LIB_PARSER_H__
#define  __ALRIGHT_LIB_PARSER_H__

#define  LIB_PARSER_VERSION      "1.0.0"
#include "common.h"
#include <stdarg.h>
#include <ctype.h>

//  Describe:
//    Product output string according to the format describe as pszFormat and 
//    write the output appends to the ith character of psz. It is the same as 
//    sprintf() if iChar is zero.
//  i/p:
//    psz       : The original string before append.
//    iChar     : The end of original string and the start of producting string
//    pszFormat : The format.
//  o/p:
//    Total lenght of the output string.
int Co_Sprintf(LPSTR psz, int iChar, LPSTR pszFormat, ...);

//	Skip character define.
typedef enum _SKIP_T
{
  SKIP_T_SPACE,
  SKIP_T_ALPHABET
}SKIP_T;

//  i/p:
//    psz : The string pointer.
//    Type: Skip character.
//    bForward:
//      TRUE  : Skip the front side.
//      FALSE : Skip the back side.
//  o/p:
//    The skipped pointer.
LPSTR Co_SkipCharacter(LPSTR pszBuf, SKIP_T Type, BOOL bForward);
#endif  //  __ALRIGHT_LIB_PARSER_H__