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
uniform float		u_HDRExposure;
uniform float		u_HDRMaxLuminance;

const vec4			LUMINANCE_VECTOR = vec4(0.2125, 0.7154, 0.0721, 0.0);
const vec3			BLUE_SHIFT_VECTOR = vec3(1.05, 0.97, 1.27); 

void	main()
{
	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 st = gl_FragCoord.st * r_FBufScale;
	
#if defined(ATI_flippedImageFix)
	// BUGFIX: the ATI driver flips the image
	st.t = 1.0 - st.t;
#endif
	
	// scale by the screen non-power-of-two-adjust
	st *= r_NPOTScale;
	
	vec4 color = texture2D(u_CurrentMap, st);
	
	// see http://www.gamedev.net/reference/articles/article2208.asp
	// for Mathematics of Reinhard's Photographic Tone Reproduction Operator
	
	// perform tone-mapping
#if 0
	color *= dot(LUMINANCE_VECTOR, color);
	color *= u_HDRExposure * (u_HDRExposure / u_HDRMaxLuminance + 1.0) / (u_HDRExposure + 1.0);

#elif 0
	color *= dot(LUMINANCE_VECTOR, color);
	float finalLuminance = u_HDRExposure * ((u_HDRExposure + 1.0) / (u_HDRMaxLuminance * u_HDRMaxLuminance)) / (u_HDRExposure + 1.0);
	color *= finalLuminance;
	
#elif 0
	float scaledLuminance = u_HDRExposure * dot(LUMINANCE_VECTOR, color);
	//float finalLuminance = scaledLuminance * ((scaledLuminance + 1.0) / (u_HDRMaxLuminance * u_HDRMaxLuminance)) / (scaledLuminance + 1.0);
	float finalLuminance = (scaledLuminance * ((scaledLuminance / (u_HDRMaxLuminance * u_HDRMaxLuminance)) + 1.0)) / (scaledLuminance + 1.0);
	color *= finalLuminance;

#elif 0
	float scaledLuminance = u_HDRExposure * dot(LUMINANCE_VECTOR, color);
	color *= scaledLuminance * (scaledLuminance / u_HDRMaxLuminance + 1.0) / (scaledLuminance + 1.0);
	
#elif 0
	float scaledLuminance = dot(LUMINANCE_VECTOR, color) * u_HDRExposure;
	color *= scaledLuminance / (scaledLuminance + 1.0);

#elif 1
	
#if 1
	// define a linear blending from -1.5 to 2.6 (log scale) which
	// determines the lerp amount for blue shift
    float blueShiftCoefficient = clamp(1.0 - (adaptedLuminance + 1.5) / 4.1, 0.0, 1.0);

	// lerp between current color and blue, desaturated copy
    vec3 rodColor = dot(color.rgb, LUMINANCE_VECTOR) * BLUE_SHIFT_VECTOR;
    color.rgb = lerp(color.rgb, rodColor, blueShiftCoefficient);
#endif
	
	color.rgb *= u_HDRExposure * dot(LUMINANCE_VECTOR, color);
	color.rgb /= (1.0 + color.rgb);
	
#else
	// exponential tone mapping
	color = 1.0 - exp(-u_HDRExposure * color * dot(LUMINANCE_VECTOR, color));

#endif
	
	gl_FragColor = color;
}
