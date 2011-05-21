#include "myendian.h"

MrtSwathEndianness GetMachineEndianness(void)
{
   long one = 1;
   if( !(*((char *)(&one))) )
      return MRTSWATH_BIG_ENDIAN;
   return MRTSWATH_LITTLE_ENDIAN;
}


