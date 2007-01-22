/*
===========================================================================
Copyright (C) 2007 Robert Beckebans <trebor_7@users.sourceforge.net>

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
uniform sampler2D	u_PositionMap;
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;
uniform vec3		u_ViewOrigin;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float       u_LightScale;
uniform mat4		u_LightAttenuationMatrix;
uniform vec2		u_FBufScale;
uniform vec2		u_NPOTScale;

void	main()
{
	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 st = gl_FragCoord.st * u_FBufScale;
	
	// scale by the screen non-power-of-two-adjust
	st *= u_NPOTScale;

	// compute normal in world space
	vec3 N = 2.0 * (texture2D(u_NormalMap, st).xyz - 0.5);
		
	// compute vertex position in world space
	vec4 P = texture2D(u_PositionMap, st).xyzw;
	
	// compute light direction in world space
	vec3 L = normalize(u_LightOrigin - P.xyz);
	
	// compute view direction in world space
	vec3 V = normalize(u_ViewOrigin - P.xyz);
	
	// compute half angle in world space
	vec3 H = normalize(L + V);
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, st);
	diffuse.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
	
	// compute the specular term
	vec4 S = texture2D(u_SpecularMap, st);
	vec3 specular = S.rgb * u_LightColor * pow(clamp(dot(N, H), 0.0, 1.0), S.a);
	
	// compute attenuation
	vec3 texAttenXYZ		= (u_LightAttenuationMatrix * vec4(P.xyz, 1.0)).xyz;
	vec3 attenuationXY		= texture2D(u_AttenuationMapXY, texAttenXYZ.xy).rgb;
	vec3 attenuationZ		= texture2D(u_AttenuationMapZ, vec2(texAttenXYZ.z, 0)).rgb;
	
	// compute final color
	vec4 color = diffuse;
	color.rgb += specular;
	color.rgb *= attenuationXY;
	color.rgb *= attenuationZ;
	color.rgb *= u_LightScale;

	gl_FragColor = color;
//	gl_FragDepth = P.w;
}
