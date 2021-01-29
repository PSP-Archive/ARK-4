/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <stdlib.h>
#include <ansi_c_functions.h>
#include "lib.h"

/*
 * Basic Sprintf functions thanks to Fanjita and Noobz
 */ 

void numtohex8(char *dst, int n)
{
   int i;
   static char hex[]="0123456789ABCDEF";
   for (i=0; i<8; i++)
   {
      dst[i]=hex[(n>>((7-i)*4))&15];
   }
}

void numtohex4(char *dst, int n)
{
   int i;
   static char hex[]="0123456789ABCDEF";
   for (i=4; i<8; i++)
   {
      dst[i-4]=hex[(n>>((7-i)*4))&15];
   }
}

void numtohex2(char *dst, int n)
{
   int i;
   static char hex[]="0123456789ABCDEF";
   for (i=6; i<8; i++)
   {
      dst[i-6]=hex[(n>>((7-i)*4))&15];
   }
}

// limited sprintf function - avoids pulling in large library
int writeFormat(char *xibuff, const char *xifmt, u32 xidata)
{
  // check for empty format
  if (xifmt[0] == '\0')
  {
    *xibuff = '?';
    return(1);
  }
  else
  {
    if ((xifmt[0] == '0') &&
        (xifmt[1] == '8') &&
        (xifmt[2] == 'l') &&
        (xifmt[3] == 'X'))
    {
      numtohex8(xibuff, xidata);
      return(8);
    }
    else if ((xifmt[0] == '0') &&
             (xifmt[1] == '4') &&
             (xifmt[2] == 'X'))
    {
      numtohex4(xibuff, xidata);
      return(4);
    }
    else if ((xifmt[0] == '0') &&
             (xifmt[1] == '2') &&
             (xifmt[2] == 'X'))
    {
      numtohex2(xibuff, xidata);
      return(2);
    }
    else if (xifmt[0] == 'c')
    {
      *xibuff = (unsigned char)xidata;
      return(1);
    }
    else if (xifmt[0] == 's')
    {
      const char *lptr   = (const char *)xidata;
      int         lcount = 0;

      /***********************************************************************/
      /* Artificially limit %s format to 150 bytes, as a cheap way to        */
      /* avoid log buffer overflow.                                          */
      /***********************************************************************/
      while ((*lptr != 0) && (lcount < 150))
      {
        *xibuff++ = *lptr++;
        lcount++;
      }
      return(lcount);
    }
    else if (xifmt[0] == 'd')
    {
      char lbuff[15];
      int  lnumbuff = 0;
      int  lcount = 0;
      int  lchar;
      int lnum   = (int)xidata;
      if (lnum < 0)
      {
        *xibuff++ = '-';
        lcount++;
        lnum = 0 - lnum;
      }

      lchar = lnum % 10;
      lbuff[0] = lchar + 48;
      lnumbuff++;
      lnum -= lchar;

      while (lnum > 0)
      {
        lnum  = lnum / 10;
        lchar = lnum % 10;
        lbuff[lnumbuff++] = lchar + 48;
        lnum -= lchar;
      }

      while (lnumbuff > 0)
      {
        *xibuff++ = lbuff[--lnumbuff];
        lcount++;
      }

      return(lcount);
    }
    else if ((xifmt[0] == 'p'))
    {
      numtohex8(xibuff, xidata);
      return(8);
    }

    return(0);
  }
}

void mysprintf11(char *xobuff, const char *xifmt,
   u32 xidata,
   u32 xidata2,
   u32 xidata3,
   u32 xidata4,
   u32 xidata5,
   u32 xidata6,
   u32 xidata7,
   u32 xidata8,
   u32 xidata9,
   u32 xidata10,
   u32 xidata11)
{
  int  lparam = 0;
  char lfmt[10];
  char *lfmtptr;

  while (*xifmt != '\0')
  {
    if (*xifmt != '%')
    {
      *xobuff++ = *xifmt++;
    }
    else
    {
      xifmt++;  // skip the %
      lfmtptr = lfmt;
      while ((*xifmt == '0')
          || (*xifmt == '2')
          || (*xifmt == '4')
          || (*xifmt == '8')
          || (*xifmt == 'l')
          || (*xifmt == 'X')
          || (*xifmt == 'd')
          || (*xifmt == 'p')
          || (*xifmt == 's')
          || (*xifmt == 'c'))
      {
        *lfmtptr ++ = *xifmt++;
      }
      *lfmtptr = '\0';

      switch (lparam)
      {
        case 0:
          xobuff += writeFormat(xobuff, lfmt, xidata);
          break;
        case 1:
          xobuff += writeFormat(xobuff, lfmt, xidata2);
          break;
        case 2:
          xobuff += writeFormat(xobuff, lfmt, xidata3);
          break;
        case 3:
          xobuff += writeFormat(xobuff, lfmt, xidata4);
          break;
        case 4:
          xobuff += writeFormat(xobuff, lfmt, xidata5);
          break;
        case 5:
          xobuff += writeFormat(xobuff, lfmt, xidata6);
          break;
        case 6:
          xobuff += writeFormat(xobuff, lfmt, xidata7);
          break;
        case 7:
          xobuff += writeFormat(xobuff, lfmt, xidata8);
          break;
        case 8:
          xobuff += writeFormat(xobuff, lfmt, xidata9);
          break;
        case 9:
          xobuff += writeFormat(xobuff, lfmt, xidata10);
          break;
        case 10:
          xobuff += writeFormat(xobuff, lfmt, xidata11);
          break;
      }
      lparam++;
    }
  }

  *xobuff = '\0';
}
void mysprintf0(char *xobuff, const char *xifmt)
{
    while (*xifmt != '\0')
    {
        *xobuff++ = *xifmt++;
    }
    *xobuff = '\0';
}
