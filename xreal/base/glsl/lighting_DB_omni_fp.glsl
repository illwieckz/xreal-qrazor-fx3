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
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;
uniform samplerCube	u_ShadowMap;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float       u_LightScale;
uniform mat4		u_ModelMatrix;

varying vec3		var_Vertex;
varying vec2		var_TexDiffuse;
varying vec2		var_TexNormal;
varying vec3		var_TexAttenXYZ;
varying mat3		var_TS2OSMatrix;

void	main()
{
	// compute light direction in world space
	vec3 L = normalize(u_LightOrigin - var_Vertex);
	
	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal).xyz - 0.5);
	
	// transform normal into object space
	N = var_TS2OSMatrix * N;
	
	// transform normal into world space
	N = normalize((u_ModelMatrix * vec4(N, 0.0)).xyz);
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, var_TexDiffuse);
	diffuse.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
	
	// compute attenuation
	vec3 attenuationXY		= texture2D(u_AttenuationMapXY, var_TexAttenXYZ.xy).rgb;
	vec3 attenuationZ		= texture2D(u_AttenuationMapZ, vec2(var_TexAttenXYZ.z, 0)).rgb;
					
	// compute final color
	vec4 color = diffuse;
	color.rgb *= attenuationXY;
	color.rgb *= attenuationZ;
	color.rgb *= u_LightScale;
	
#if defined(SHADOWMAPPING)
	// compute incident ray
	vec3 I = var_Vertex - u_LightOrigin;
#if 1
	float vertexDistance = length(I) / u_LightRadius - 0.005f;
	vec4 extract = float4(1.0, 0.00390625, 0.0000152587890625, 0.000000059604644775390625);
	float shadowDistance = dot(textureCube(u_ShadowMap, I), extract);
	float shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
	color.rgb *= shadow;
#else
	color.rgb *= textureCube(u_ShadowMap, I).rgb;
#endif
#endif

	gl_FragColor = color;
}
