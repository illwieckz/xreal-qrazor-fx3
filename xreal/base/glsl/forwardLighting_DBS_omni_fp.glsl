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
#if defined(VSM)
uniform samplerCube	u_ShadowMap;
#endif
uniform vec3		u_ViewOrigin;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float       u_LightScale;
uniform int			u_ShadowCompare;
uniform float       u_ShadowTexelSize;
uniform float		u_SpecularExponent;

varying vec3		var_Vertex;
varying vec4		var_TexDiffuse;
varying vec4		var_TexNormal;
varying vec2		var_TexSpecular;
varying vec3		var_TexAttenXYZ;
varying mat3		var_TangentToWorldMatrix;
//varying vec4		var_Color;


void	main()
{
	float shadow = 1.0;

#if defined(VSM)
	if(bool(u_ShadowCompare))
	{
		// compute incident ray
		vec3 I0 = var_Vertex - u_LightOrigin;
	
		#if defined(PCF_2X2)
		// 2x2 PCF
		float offsetScale = u_ShadowTexelSize;
	
		vec3 I1 = I0 + vec3( 2.0,-1.0, 1.0) * offsetScale;
		vec3 I2 = I0 + vec3( 1.0, 2.0,-1.0) * offsetScale;
		vec3 I3 = I0 + vec3(-1.0, 1.0, 2.0) * offsetScale;
		
		vec4 shadowMap = textureCube(u_ShadowMap, I0);
		shadowMap += textureCube(u_ShadowMap, I1);
		shadowMap += textureCube(u_ShadowMap, I2);
		shadowMap += textureCube(u_ShadowMap, I3);
		shadowMap *= 0.25;
	
		#elif defined(PCF_3X3)
		// 3x3 PCF
		float offsetScale = 0.3; //u_ShadowTexelSize * 100;
	
		vec3 I1 = I0 + vec3( 2.0,-1.0, 1.0) * offsetScale;
		vec3 I2 = I0 + vec3( 1.0, 2.0,-1.0) * offsetScale;
		vec3 I3 = I0 + vec3(-1.0, 1.0, 2.0) * offsetScale;
		vec3 I4 = I0 + I1;
		vec3 I5 = I0 + I2;
		vec3 I6 = I0 + I3;
		vec3 I7 = I1 + I2;
		vec3 I8 = I2 + I3;
		
		vec4 shadowMap = textureCube(u_ShadowMap, I0);
		shadowMap += textureCube(u_ShadowMap, I1);
		shadowMap += textureCube(u_ShadowMap, I2);
		shadowMap += textureCube(u_ShadowMap, I3);
		shadowMap += textureCube(u_ShadowMap, I4);
		shadowMap += textureCube(u_ShadowMap, I5);
		shadowMap += textureCube(u_ShadowMap, I6);
		shadowMap += textureCube(u_ShadowMap, I7);
		shadowMap += textureCube(u_ShadowMap, I8);
		shadowMap *= 0.11111111;
		#else
		vec4 shadowMap = textureCube(u_ShadowMap, I0);
		#endif

		const float	SHADOW_BIAS = 0.0;//01;
		float vertexDistance = length(I0) / u_LightRadius - SHADOW_BIAS;
	
		#if defined(VSM_CLAMP)
		// convert to [-1, 1] vector space
		shadowMap = 2.0 * (shadowMap - 0.5);
		#endif
	
		float shadowDistance = shadowMap.r;
		float shadowDistanceSquared = shadowMap.g;
	
		// standard shadow map comparison
		shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
	
		// variance shadow mapping
		float E_x2 = shadowDistanceSquared;
		float Ex_2 = shadowDistance * shadowDistance;
	
		// AndyTX: VSM_EPSILON is there to avoid some ugly numeric instability with fp16
		float variance = min(max(E_x2 - Ex_2, 0.0) + VSM_EPSILON, 1.0);
	
		float mD = shadowDistance - vertexDistance;
		float mD_2 = mD * mD;
		float p = variance / (variance + mD_2);
	
		#if defined(DEBUG_VSM)
		gl_FragColor.r = DEBUG_VSM & 1 ? variance : 0.0;
		gl_FragColor.g = DEBUG_VSM & 2 ? mD_2 : 0.0;
		gl_FragColor.b = DEBUG_VSM & 4 ? p : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#else
		shadow = max(shadow, p);
		#endif
	}
	
	if(shadow <= 0.0)
	{
		discard;
	}
	else
#endif
	{
		// compute view direction in world space
		vec3 V = normalize(u_ViewOrigin - var_Vertex);
	
		// compute light direction in world space
		vec3 L = normalize(u_LightOrigin - var_Vertex);
	
		// compute half angle in world space
		vec3 H = normalize(L + V);
	
		// compute normal in tangent space from normalmap
		vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal.st).xyz - 0.5);
		N.z *= r_NormalScale;
		normalize(N);
	
		// transform normal into world space
		N = var_TangentToWorldMatrix * N;
	
		// compute the diffuse term
		vec4 diffuse = texture2D(u_DiffuseMap, var_TexDiffuse.st);
		diffuse.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
	
		// compute the specular term
		vec3 specular = texture2D(u_SpecularMap, var_TexSpecular).rgb * u_LightColor * pow(clamp(dot(N, H), 0.0, 1.0), u_SpecularExponent) * r_SpecularScale;
	
		// compute attenuation
		vec3 attenuationXY		= texture2D(u_AttenuationMapXY, var_TexAttenXYZ.xy).rgb;
		vec3 attenuationZ		= texture2D(u_AttenuationMapZ, vec2(var_TexAttenXYZ.z, 0)).rgb;
					
		// compute final color
		vec4 color = diffuse;
		color.rgb += specular;
		color.rgb *= attenuationXY;
		color.rgb *= attenuationZ;
		color.rgb *= u_LightScale;
		color.rgb *= shadow;
	
		color.r *= var_TexDiffuse.p;
		color.gb *= var_TexNormal.pq;

		gl_FragColor = color;
	}
}
