///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////
// Primary authors:
//     Florian Kainz <kainz@ilm.com>
//     Rod Bogart <rgb@ilm.com>

//---------------------------------------------------------------------------
//
//	half -- a 16-bit floating point number class:
//
//	Type half can represent positive and negative numbers whose
//	magnitude is between roughly 6.1e-5 and 6.5e+4 with a relative
//	error of 9.8e-4; numbers smaller than 6.1e-5 can be represented
//	with an absolute error of 6.0e-8.  All integers from -2048 to
//	+2048 can be represented exactly.
//
//	Type half behaves (almost) like the built-in C++ floating point
//	types.  In arithmetic expressions, half, float and double can be
//	mixed freely.  Here are a few examples:
//
//	    half a (3.5);
//	    float b (a + sqrt (a));
//	    a += b;
//	    b += a;
//	    b = a + 7;
//
//	Conversions from half to float are lossless; all half numbers
//	are exactly representable as floats.
//
//	Conversions from float to half may not preserve the float's
//	value exactly.  If a float is not representable as a half, the
//	float value is rounded to the nearest representable half.  If
//	a float value is exactly in the middle between the two closest
//	representable half values, then the float value is rounded to
//	the half with the greater magnitude.
//
//	Overflows during float-to-half conversions cause arithmetic
//	exceptions.  An overflow occurs when the float value to be
//	converted is too large to be represented as a half, or if the
//	float value is an infinity or a NAN.
//
//	The implementation of type half makes the following assumptions
//	about the implementation of the built-in C++ types:
//
//	    float is an IEEE 754 single-precision number
//	    sizeof (float) == 4
//	    sizeof (unsigned int) == sizeof (float)
//	    alignof (unsigned int) == alignof (float)
//	    sizeof (unsigned short) == 2
//
//---------------------------------------------------------------------------


// tr_image_exr.c

#ifdef __cplusplus
extern "C"
{
#endif
#include "tr_local.h"
#ifdef __cplusplus
}
#endif

#include <OpenEXR/half.h>


#ifdef __cplusplus
extern          "C"
{
#endif

void LoadRGBEToFloats(const char *name, float **pic, int *width, int *height, qboolean doGamma, qboolean toneMap, qboolean compensate);

void LoadRGBEToHalfs(const char *name, unsigned short ** halfImage, int *width, int *height)
{
	int             i, j;
	int             w, h;
	float          *hdrImage;
	float          *floatbuf;
	unsigned short *halfbuf;

#if 0
	w = h = 0;
	LoadRGBEToFloats(name, &hdrImage, &w, &h, qtrue, qtrue, qtrue);

	*width = w;
	*height = h;

	*ldrImage = ri.Malloc(w * h * 4);
	pixbuf = *ldrImage;

	floatbuf = hdrImage;
	for(i = 0; i < (w * h); i++)
	{
		for(j = 0; j < 3; j++)
		{
			sample[j] = *floatbuf++;
		}

		NormalizeColor(sample, sample);

		*pixbuf++ = (byte) (sample[0] * 255);
		*pixbuf++ = (byte) (sample[1] * 255);
		*pixbuf++ = (byte) (sample[2] * 255);
		*pixbuf++ = (byte) 255;
	}
#else
	w = h = 0;
	LoadRGBEToFloats(name, &hdrImage, &w, &h, qtrue, qfalse, qtrue);

	*width = w;
	*height = h;

	*halfImage = (unsigned short *) Com_Allocate(w * h * 3 * 6);

	halfbuf = *halfImage;
	floatbuf = hdrImage;
	for(i = 0; i < (w * h); i++)
	{
		for(j = 0; j < 3; j++)
		{
			half sample(*floatbuf++);
			*halfbuf++ = sample.bits();
		}
	}
#endif

	Com_Dealloc(hdrImage);
}

#ifdef __cplusplus
}
#endif
