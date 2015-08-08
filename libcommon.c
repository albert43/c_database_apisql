//    libcommon.c
#include "libcommon.h"

int Co_Sprintf(LPSTR psz, int iChar, LPSTR pszFormat, ...)
{
	va_list  ap;

	va_start (ap, pszFormat);
	iChar = vsprintf (psz + iChar, pszFormat, ap);
	va_end (ap);
	
	return iChar;
}

LPSTR Co_SkipCharacter(LPSTR pszBuf, SKIP_T Type, BOOL bForward)
{
  LPSTR   psz = pszBuf;
  int     iDirect; 
  
  if (psz)
  {
    if (bForward)
      iDirect = 1;
    else
      iDirect = -1;
      
    if (Type == SKIP_T_SPACE)
    {
      while (psz[0] == ' ')
        psz += iDirect;
    }
    else if (Type == SKIP_T_ALPHABET)
    {
      while (isalpha(psz[0]))
        psz += iDirect;
    }
    //  Unsupport type.
    else
    {
      psz = NULL;
    }
  }
  
  return psz;
}