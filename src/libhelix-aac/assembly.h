/* ***** BEGIN LICENSE BLOCK *****  
 * Source last modified: $Id: assembly.h,v 1.7 2005/11/10 00:04:40 margotm Exp $ 
 *   
 * Portions Copyright (c) 1995-2005 RealNetworks, Inc. All Rights Reserved.  
 *       
 * The contents of this file, and the files included with this file, 
 * are subject to the current version of the RealNetworks Public 
 * Source License (the "RPSL") available at 
 * http://www.helixcommunity.org/content/rpsl unless you have licensed 
 * the file under the current version of the RealNetworks Community 
 * Source License (the "RCSL") available at 
 * http://www.helixcommunity.org/content/rcsl, in which case the RCSL 
 * will apply. You may also obtain the license terms directly from 
 * RealNetworks.  You may not use this file except in compliance with 
 * the RPSL or, if you have a valid RCSL with RealNetworks applicable 
 * to this file, the RCSL.  Please see the applicable RPSL or RCSL for 
 * the rights, obligations and limitations governing use of the 
 * contents of the file. 
 *   
 * This file is part of the Helix DNA Technology. RealNetworks is the 
 * developer of the Original Code and owns the copyrights in the 
 * portions it created. 
 *   
 * This file, and the files included with this file, is distributed 
 * and made available on an 'AS IS' basis, WITHOUT WARRANTY OF ANY 
 * KIND, EITHER EXPRESS OR IMPLIED, AND REALNETWORKS HEREBY DISCLAIMS 
 * ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET 
 * ENJOYMENT OR NON-INFRINGEMENT. 
 *  
 * Technology Compatibility Kit Test Suite(s) Location:  
 *    http://www.helixcommunity.org/content/tck  
 *  
 * Contributor(s):  
 *   
 * ***** END LICENSE BLOCK ***** */  

/**************************************************************************************
 * Fixed-point HE-AAC decoder
 * Jon Recker (jrecker@real.com)
 * February 2005
 *
 * assembly.h - inline assembly language functions and prototypes
 *
 * MULSHIFT32(x, y) 		signed multiply of two 32-bit integers (x and y), 
 *                            returns top 32-bits of 64-bit result
 * CLIPTOSHORT(x)			convert 32-bit integer to 16-bit short, 
 *                            clipping to [-32768, 32767]
 * FASTABS(x)               branchless absolute value of signed integer x
 * CLZ(x)                   count leading zeros on signed integer x
 * MADD64(sum64, x, y)		64-bit multiply accumulate: sum64 += (x*y)
 **************************************************************************************/

#ifndef _ASSEMBLY_H
#define _ASSEMBLY_H

typedef long long Word64;

static __inline__ int MULSHIFT32(int x, int y)
{
    int z;

    z = (Word64)x * (Word64)y >> 32;
    
	return z;
}

static __inline short CLIPTOSHORT(int x)
{
	int sign;

	/* clip to [-32768, 32767] */
	sign = x >> 31;
	if (sign != (x >> 15))
		x = sign ^ ((1 << 15) - 1);

	return (short)x;
}

static __inline int FASTABS(int x) 
{
	int sign;

	sign = x >> (sizeof(int) * 8 - 1);
	x ^= sign;
	x -= sign;

	return x;
}

static __inline int CLZ(int x)
{
	int numZeros;

	if (!x)
		return 32;

	/* count leading zeros with binary search (function should be 17 ARM instructions total) */
	numZeros = 1;
	if (!((unsigned int)x >> 16))	{ numZeros += 16; x <<= 16; }
	if (!((unsigned int)x >> 24))	{ numZeros +=  8; x <<=  8; }
	if (!((unsigned int)x >> 28))	{ numZeros +=  4; x <<=  4; }
	if (!((unsigned int)x >> 30))	{ numZeros +=  2; x <<=  2; }

	numZeros -= ((unsigned int)x >> 31);

	return numZeros;
}

typedef union _U64 {
	Word64 w64;
	struct {
#ifdef __XTENSA__		
		unsigned int lo32;
		signed int   hi32;
#else
		/* PowerPC = big endian */
		signed int   hi32;
		unsigned int lo32;
#endif		
	} r;
} U64;

static __inline Word64 MADD64(Word64 sum64, int x, int y)
{
	sum64 += (Word64)x * (Word64)y;

	return sum64;
}

/* From coder.h, ORIGINAL:
clip to [-2^n, 2^n-1], valid range of n = [1, 30]
//TODO (FB) Is there a better way ?
*/
#define CLIP_2N(y, n) { \
	int sign = (y) >> 31;  \
	if (sign != (y) >> (n))  { \
		(y) = sign ^ ((1 << (n)) - 1); \
	} \
}

/* From coder.h, ORIGINAL:
 do y <<= n, clipping to range [-2^30, 2^30 - 1] (i.e. output has one guard bit) 
*/
//TODO (FB) Is there a better way ?
#define CLIP_2N_SHIFT(y, n) {                   \
        int sign = (y) >> 31;                   \
        if (sign != (y) >> (30 - (n)))  {       \
            (y) = sign ^ (0x3fffffff);          \
        } else {                                \
            (y) = (y) << (n);                   \
        }                                       \
    }



//#define FASTABS(x) abs(x) //FB
//#define CLZ(x) __builtin_clz(x) //FB

#ifndef CLIP_2N
#define CLIP_2N(y, n) { \
        int sign = (y) >> 31;  \
        if (sign != (y) >> (n))  { \
                (y) = sign ^ ((1 << (n)) - 1); \
        } \
}
#endif

#ifndef CLIP_2N_SHIFT
/* From coder.h, ORIGINAL:
 do y <<= n, clipping to range [-2^30, 2^30 - 1] (i.e. output has one guard bit) 
*/
//TODO (FB) Is there a better way ?
#define CLIP_2N_SHIFT(y, n) {                   \
        int sign = (y) >> 31;                   \
        if (sign != (y) >> (30 - (n)))  {       \
            (y) = sign ^ (0x3fffffff);          \
        } else {                                \
            (y) = (y) << (n);                   \
        }                                       \
    }
#endif

#endif /* _ASSEMBLY_H */
