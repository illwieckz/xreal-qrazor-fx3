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
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;
uniform sampler2D	u_ShadowMap;
uniform vec3		u_ViewOrigin;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float		u_LightScale;
uniform int			u_ShadowCompare;
uniform float		u_SpecularExponent;
uniform mat4		u_ModelMatrix;

varying vec3		var_Vertex;
varying vec4		var_TexDiffuse;
varying vec4		var_TexNormal;
varying vec2		var_TexSpecular;
varying vec4		var_TexAtten;
varying vec4		var_TexShadow;
varying mat3		var_TangentToWorldMatrix;

void	main()
{
	if(var_TexAtten.q <= 0.0)
	{
		discard;
	}
	
	// compute view direction in world space
	vec3 V = normalize(u_ViewOrigin - var_Vertex);
	
	// compute light direction in world space
	vec3 L = normalize(u_LightOrigin - var_Vertex);
	
	// compute half angle in world space
	vec3 H = normalize(L + V);
	
	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal.st).xyz - 0.5);

	// transform normal into world space
	N = var_TangentToWorldMatrix * N;
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, var_TexDiffuse.st);
	diffuse.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
	
	// compute the specular term
	vec3 specular = texture2D(u_SpecularMap, var_TexSpecular).rgb * u_LightColor * pow(clamp(dot(N, H), 0.0, 1.0), u_SpecularExponent);
	
	// compute attenuation
	vec3 attenuationXY = texture2DProj(u_AttenuationMapXY, var_TexAtten.xyw).rgb;
	vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(1.0 - var_TexAtten.z, 0.0)).rgb;

	// compute final color
	vec4 color = diffuse;
	color.rgb += specular;
	color.rgb *= attenuationXY;
	color.rgb *= attenuationZ;
	color.rgb *= u_LightScale;
	
	color.r *= var_TexDiffuse.p;
	color.gb *= var_TexNormal.pq;

#if defined(VSM)
	if(bool(u_ShadowCompare))
	{
		float vertexDistance = length(var_Vertex - u_LightOrigin) / u_LightRadius;
		vec2 shadowDistances = texture2DProj(u_ShadowMap, var_TexShadow.xyw).rg;
	
		// standard shadow map comparison
		float shadow = vertexDistance <= shadowDistances.r ? 1.0 : 0.0;
	
		// variance shadow mapping
		float E_x2 = shadowDistances.g;
		float Ex_2 = shadowDistances.r * shadowDistances.r;
	
		// AndyTX: VSM_EPSILON is there to avoid some ugly numeric instability with fp16
		const float	VSM_EPSILON = 0.0001;
		float variance = min(max(E_x2 - Ex_2, 0.0) + VSM_EPSILON, 1.0);
	
		float mD = shadowDistances.r - vertexDistance;
		float pMax = variance / (variance + mD * mD);
	
		color.rgb *= max(shadow, pMax);
	}
#endif
	
	gl_FragColor = color;
}
