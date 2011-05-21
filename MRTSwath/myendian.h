#ifndef MRTSWATH_ENDIAN_H
#define MRTSWATH_ENDIAN_H

/* When a raw binary file is read output, specify the endianness.  Default is
   to use the endianness of the machine.
 */
typedef enum
{
    MRTSWATH_UNKNOWN_ENDIAN, MRTSWATH_BIG_ENDIAN, MRTSWATH_LITTLE_ENDIAN
} MrtSwathEndianness;

MrtSwathEndianness GetMachineEndianness(void);

#endif

