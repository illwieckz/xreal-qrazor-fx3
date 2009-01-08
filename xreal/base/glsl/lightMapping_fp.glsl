/*
===========================================================================
Copyright (C) 2008 Robert Beckebans <trebor_7@users.sourceforge.net>

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

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_LightMap;
uniform float		u_AlphaTest;

varying vec2		var_TexDiffuse;
varying vec2		var_TexLight;


#if defined(r_HDRRendering)
vec3 DecodeRGBE(vec4 rgbe)
{
	vec3 decoded;
	//float fExp = rgbe.a * 255 - 255;
	//float fExp = rgbe.a * 255 - (128 + 8);
	float fExp = 0.0;
	decoded = rgbe.rgb * exp2(fExp);
  
	return decoded;
}
#endif

void	main()
{
	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, var_TexDiffuse);
	if(diffuse.a <= u_AlphaTest)
	{
		discard;
		return;
	}

#if defined(r_showLightMaps)

#if 0 //defined(r_HDRRendering)
	gl_FragColor = vec4(DecodeRGBE(texture2D(u_LightMap, var_TexLight)), 1.0);
#else
	gl_FragColor = texture2D(u_LightMap, var_TexLight);
#endif

#else

	// compute light color from object space lightmap
#if 0 //defined(r_HDRRendering)
	vec3 lightColor = DecodeRGBE(texture2D(u_LightMap, var_TexLight));
#else
	vec3 lightColor = texture2D(u_LightMap, var_TexLight).rgb;
#endif
	
	diffuse.rgb *= lightColor;
	
	gl_FragColor = diffuse;
#endif
}
