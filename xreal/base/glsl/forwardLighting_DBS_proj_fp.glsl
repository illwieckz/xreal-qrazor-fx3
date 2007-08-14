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
uniform float       u_ShadowTexelSize;
uniform float		u_SpecularExponent;
uniform mat4		u_ModelMatrix;

varying vec4		var_Vertex;
varying vec4		var_TexDiffuse;
varying vec4		var_TexNormal;
varying vec2		var_TexSpecular;
varying vec4		var_TexAtten;
varying vec4		var_Tangent;
varying vec4		var_Binormal;
varying vec4		var_Normal;


float linstep(float min, float max, float v)
{
    return clamp((v - min) / (max - min), 0.0, 1.0);
}

#if defined(VSM)
vec4 PCF(vec3 projectiveBiased, float filterWidth, float samples)
{
	// compute step size for iterating through the kernel
	float stepSize = 2.0 * filterWidth / samples;
	
	vec4 moments = vec4(0.0, 0.0, 0.0, 0.0);
	for(float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for(float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			moments += texture2D(u_ShadowMap, projectiveBiased.xy + vec2(i, j));
		}
	}
	
	// return average of the samples
	moments *= (1.0 / (samples * samples));
	return moments;
}
#endif

void	main()
{
	float shadow = 1.0;

	if(var_TexAtten.q <= 0.0)
	{
		discard;
	}
	
#if defined(VSM)
	if(bool(u_ShadowCompare))
	{
		vec4 texShadow;
		texShadow.s = var_Vertex.w;
		texShadow.t = var_Tangent.w;
		texShadow.p = var_Binormal.w;
		texShadow.q = var_Normal.w;
		
		#if defined(PCF_2X2)
		vec4 shadowMoments = PCF(texShadow.xyz / texShadow.w, u_ShadowTexelSize * 1.3, 2.0);
		#elif defined(PCF_3X3)
		vec4 shadowMoments = PCF(texShadow.xyz / texShadow.w, u_ShadowTexelSize * 1.3, 3.0);
		#elif defined(PCF_4X4)
		vec4 shadowMoments = PCF(texShadow.xyz / texShadow.w, u_ShadowTexelSize * 1.3, 4.0);
		#else
		vec4 shadowMoments = texture2DProj(u_ShadowMap, texShadow.xyw);
		#endif
	
		#if defined(VSM_CLAMP)
		// convert to [-1, 1] vector space
		shadowMoments = 2.0 * (shadowMoments - 0.5);
		#endif
		
		float shadowDistance = shadowMoments.r;
		float shadowDistanceSquared = shadowMoments.g;
		
		const float	SHADOW_BIAS = 0.001;
		float vertexDistance = length(var_Vertex.xyz - u_LightOrigin) / u_LightRadius - SHADOW_BIAS;
	
		// standard shadow map comparison
		shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
	
		// variance shadow mapping
		float E_x2 = shadowDistanceSquared;
		float Ex_2 = shadowDistance * shadowDistance;
	
		// AndyTX: VSM_EPSILON is there to avoid some ugly numeric instability with fp16
		float variance = min(max(E_x2 - Ex_2, 0.0) + VSM_EPSILON, 1.0);
		//float variance = smoothstep(VSM_EPSILON, 1.0, max(E_x2 - Ex_2, 0.0));
	
		float mD = shadowDistance - vertexDistance;
		float mD_2 = mD * mD;
		float p = variance / (variance + mD_2);
		//p = linstep(0.0, 1.0, p);
		p = smoothstep(0.0, 1.0, p);
	
		#if defined(DEBUG_VSM)
		gl_FragColor.r = DEBUG_VSM & 1 ? variance : 0.0;
		gl_FragColor.g = DEBUG_VSM & 2 ? mD_2 : 0.0;
		gl_FragColor.b = DEBUG_VSM & 4 ? p : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#else
		shadow = max(shadow, p);
		//shadow = linstep(0.0, p, shadow);
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
		vec3 V = normalize(u_ViewOrigin - var_Vertex.xyz);
	
		// compute light direction in world space
		vec3 L = normalize(u_LightOrigin - var_Vertex.xyz);
	
		// compute half angle in world space
		vec3 H = normalize(L + V);
	
		// compute normal in tangent space from normalmap
		vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal.st).xyz - 0.5);
		N.z *= r_NormalScale;
		normalize(N);
	
		mat3 tangentToWorldMatrix = mat3(var_Tangent.xyz, var_Binormal.xyz, var_Normal.xyz);

		// transform normal into world space
		N = tangentToWorldMatrix * N;
	
		// compute the diffuse term
		vec4 diffuse = texture2D(u_DiffuseMap, var_TexDiffuse.st);
		diffuse.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
	
		// compute the specular term
		vec3 specular = texture2D(u_SpecularMap, var_TexSpecular).rgb * u_LightColor * pow(clamp(dot(N, H), 0.0, 1.0), u_SpecularExponent) * r_SpecularScale;
	
		// compute attenuation
		#if 0
		vec3 texAtten = var_TexAtten.xyz / var_TexAtten.w;
		vec3 attenuationXY = texture2D(u_AttenuationMapXY, texAtten.xy).rgb;
		vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(texAtten.z, 0.0)).rgb;
		#else
		vec3 attenuationXY = texture2DProj(u_AttenuationMapXY, var_TexAtten.xyw).rgb;
		vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(1.0 - var_TexAtten.z, 0.0)).rgb;
		#endif

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
