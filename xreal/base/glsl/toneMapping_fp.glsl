/*
===========================================================================
Copyright (C) 2008-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

uniform sampler2D	u_CurrentMap;
uniform float		u_HDRKey;
uniform float		u_HDRAverageLuminance;
uniform float		u_HDRMaxLuminance;

const vec4			LUMINANCE_VECTOR = vec4(0.2125, 0.7154, 0.0721, 0.0);
const vec3			BLUE_SHIFT_VECTOR = vec3(1.05, 0.97, 1.27); 

void	main()
{
	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 st = gl_FragCoord.st * r_FBufScale;
	
	// scale by the screen non-power-of-two-adjust
	st *= r_NPOTScale;
	
	vec4 color = texture2D(u_CurrentMap, st);
	
	// see http://www.gamedev.net/reference/articles/article2208.asp
	// for Mathematics of Reinhard's Photographic Tone Reproduction Operator
	
	// get the luminance of the current pixel
	float Y = dot(LUMINANCE_VECTOR, color);
	
	// calculate the relative luminance
	float Yr = u_HDRKey * Y / u_HDRAverageLuminance;

	float Ymax = u_HDRMaxLuminance;

#if defined(r_HDRToneMappingOperator_0)
	
	// simple tone map operator
	float L = Yr / (1.0 + Yr);
	
#elif defined(r_HDRToneMappingOperator_1)
	
	float L = 1.0 - exp(-Yr);

#elif defined(r_HDRToneMappingOperator_2)
	
	float L = Yr / (1.0 + Yr) * (1.0 + Yr / (Ymax * Ymax));
	
#else
	
	// recommended by Wolgang Engel
	float L = Yr * (1.0 + Yr / (Ymax * Ymax)) / (1.0 + Yr);
#endif
	
	color.rgb *= L;
	
	gl_FragColor = color;
}
