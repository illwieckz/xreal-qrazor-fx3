/*
===========================================================================
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

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
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_LightMap;
uniform sampler2D	u_DeluxeMap;
uniform float		u_AlphaTest;
uniform vec3		u_ViewOrigin;

varying vec3		var_Vertex;
varying vec2		var_TexDiffuse;
varying vec2		var_TexNormal;
varying vec2		var_TexSpecular;
varying vec2		var_TexLight;
varying mat3		var_OS2TSMatrix;


#if defined(r_HDRRendering)
vec3 DecodeRGBE(vec4 rgbe)
{
	vec3 decoded;
	if(rgbe.a > 0.0)
	{
		float fExp = ((rgbe.a * 256.0) - 128.0) / 10.0;
		decoded = rgbe.rgb * exp2(fExp);
		//decoded = vec3(rgbe.a, rgbe.a, rgbe.a);
	}
	else
	{
		decoded = vec3(0.0, 0.0, 0.0);
	}
  
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
	gl_FragColor = texture2D(u_LightMap, var_TexLight) / 200.0;
#endif

#elif defined(r_showDeluxeMaps)
	gl_FragColor = texture2D(u_DeluxeMap, var_TexLight);
#else

	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal).xyz - 0.5);
	N = normalize(N);
	
	// compute view direction in tangent space
	vec3 V = normalize(var_OS2TSMatrix * (u_ViewOrigin - var_Vertex));
	
	// compute light direction from object space deluxe map into tangent space
	vec3 L = normalize(var_OS2TSMatrix * (2.0 * (texture2D(u_DeluxeMap, var_TexLight).xyz - 0.5)));
	
	// compute half angle in tangent space
	vec3 H = normalize(L + V);
	
	// compute light color from object space lightmap
#if 0 //defined(r_HDRRendering)
	//vec3 lightColor = DecodeRGBE(texture2D(u_LightMap, var_TexLight));
	//vec3 lightColor = pow(texture2D(u_LightMap, var_TexLight), 1.0 / 2.2);
#else
	vec3 lightColor = texture2D(u_LightMap, var_TexLight).rgb;
#endif
	
	diffuse.rgb *= lightColor.rgb * clamp(dot(N, L), 0.0, 1.0);
	
	// compute the specular term
	vec3 specular = texture2D(u_SpecularMap, var_TexSpecular).rgb * lightColor * pow(clamp(dot(N, H), 0.0, 1.0), r_SpecularExponent) * r_SpecularScale;
	
	// compute final color
	gl_FragColor.rgba = diffuse;
	gl_FragColor.rgb += specular;
#endif
	
}
