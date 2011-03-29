/*
===========================================================================
Copyright (C) 2007-2011 Robert Beckebans <trebor_7@users.sourceforge.net>

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

/* forwardLighting_fp.glsl */

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_SpecularMap;
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;
uniform samplerCube	u_ShadowMap;

uniform vec3		u_ViewOrigin;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float       u_LightScale;
uniform	float		u_LightWrapAround;
uniform int			u_ShadowCompare;
uniform float       u_ShadowTexelSize;
uniform float       u_ShadowBlur;
uniform int         u_PortalClipping;
uniform vec4		u_PortalPlane;

uniform float		u_DepthScale;

varying vec3		var_Position;
varying vec4		var_TexDiffuse;
varying vec4		var_TexNormal;
#if defined(USE_NORMAL_MAPPING)
varying vec2		var_TexSpecular;
#endif
varying vec3		var_TexAttenXYZ;
#if defined(USE_NORMAL_MAPPING)
varying vec4		var_Tangent;
varying vec4		var_Binormal;
#endif
varying vec4		var_Normal;
//varying vec4		var_Color;


#if defined(VSM) || defined(ESM)

/*
================
MakeNormalVectors

Given a normalized forward vector, create two
other perpendicular vectors
================
*/
void MakeNormalVectors(const vec3 forward, inout vec3 right, inout vec3 up)
{
	// this rotate and negate guarantees a vector
	// not colinear with the original
	right.y = -forward.x;
	right.z = forward.y;
	right.x = forward.z;

	float d = dot(right, forward);
	right += forward * -d;
	normalize(right);
	up = cross(right, forward);	// GLSL cross product is the same as in Q3A
}

vec4 PCF(vec3 I, float filterWidth, float samples)
{
	vec3 forward, right, up;
	
	forward = normalize(I);
	MakeNormalVectors(forward, right, up);

	// compute step size for iterating through the kernel
	float stepSize = 2.0 * filterWidth / samples;
	
	vec4 moments = vec4(0.0, 0.0, 0.0, 0.0);
	for(float i = -filterWidth; i < filterWidth; i += stepSize)
	{
		for(float j = -filterWidth; j < filterWidth; j += stepSize)
		{
			moments += textureCube(u_ShadowMap, I + right * i + up * j);
		}
	}
	
	// return average of the samples
	moments *= (1.0 / (samples * samples));
	return moments;
}
#endif

void	main()
{
#if defined(USE_PORTAL_CLIPPING)
	{
		float dist = dot(var_Position.xyz, u_PortalPlane.xyz) - u_PortalPlane.w;
		if(dist < 0.0)
		{
			discard;
			return;
		}
	}
#endif

	float shadow = 1.0;
#if defined(USE_SHADOWING)
#if defined(VSM)
	{
		// compute incident ray
		vec3 I = var_Position - u_LightOrigin;
	
		#if defined(PCF_2X2)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 2.0);
		#elif defined(PCF_3X3)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 3.0);
		#elif defined(PCF_4X4)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 4.0);
		#elif defined(PCF_5X5)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 5.0);
		#elif defined(PCF_6X6)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 6.0);
		#else
		vec4 shadowMoments = textureCube(u_ShadowMap, I);
		#endif
		
		#if defined(VSM_CLAMP)
		// convert to [-1, 1] vector space
		shadowMoments = 2.0 * (shadowMoments - 0.5);
		#endif
	
		float shadowDistance = shadowMoments.r;
		float shadowDistanceSquared = shadowMoments.a;
		
		const float	SHADOW_BIAS = 0.001;
		float vertexDistance = length(I) / u_LightRadius - SHADOW_BIAS;
	
		// standard shadow map comparison
		shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
	
		// variance shadow mapping
		float E_x2 = shadowDistanceSquared;
		float Ex_2 = shadowDistance * shadowDistance;
	
		// AndyTX: VSM_EPSILON is there to avoid some ugly numeric instability with fp16
		float variance = max(E_x2 - Ex_2, VSM_EPSILON);
	
		// compute probabilistic upper bound
		float mD = shadowDistance - vertexDistance;
		float mD_2 = mD * mD;
		float p = variance / (variance + mD_2);
		
		#if defined(r_LightBleedReduction)
		p = smoothstep(r_LightBleedReduction, 1.0, p);
		#endif
	
		#if defined(DEBUG_VSM)
		#extension GL_EXT_gpu_shader4 : enable
		gl_FragColor.r = (DEBUG_VSM & 1) != 0 ? variance : 0.0;
		gl_FragColor.g = (DEBUG_VSM & 2) != 0 ? mD_2 : 0.0;
		gl_FragColor.b = (DEBUG_VSM & 4) != 0 ? p : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#else
		shadow = max(shadow, p);
		#endif
	}
#elif defined(ESM)
	{
		// compute incident ray
		vec3 I = var_Position - u_LightOrigin;
	
		#if defined(PCF_2X2)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 2.0);
		#elif defined(PCF_3X3)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 3.0);
		#elif defined(PCF_4X4)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 4.0);
		#elif defined(PCF_5X5)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 5.0);
		#elif defined(PCF_6X6)
		vec4 shadowMoments = PCF(I, u_ShadowTexelSize * u_ShadowBlur * length(I), 6.0);
		#else
		vec4 shadowMoments = textureCube(u_ShadowMap, I);
		#endif
		
		const float	SHADOW_BIAS = 0.001;
		float vertexDistance = (length(I) / u_LightRadius) * r_ShadowMapDepthScale;// - SHADOW_BIAS;
		
		float shadowDistance = shadowMoments.a;
		
		// exponential shadow mapping
		shadow = clamp(exp(r_OverDarkeningFactor * (shadowDistance - vertexDistance)), 0.0, 1.0);
		
		#if defined(DEBUG_ESM)
		#extension GL_EXT_gpu_shader4 : enable
		gl_FragColor.r = (DEBUG_ESM & 1) != 0 ? shadowDistance : 0.0;
		gl_FragColor.g = (DEBUG_ESM & 2) != 0 ? -(shadowDistance - vertexDistance) : 0.0;
		gl_FragColor.b = (DEBUG_ESM & 4) != 0 ? shadow : 0.0;
		gl_FragColor.a = 1.0;
		return;
		#endif
	}
#endif // ESM

	if(shadow <= 0.0)
	{
		discard;
		return;
	}

#endif // USE_SHADOWING
	
	// compute light direction in world space
	vec3 L = normalize(u_LightOrigin - var_Position);

	vec2 texDiffuse = var_TexDiffuse.st;
	
#if defined(USE_NORMAL_MAPPING)

	// invert tangent space for twosided surfaces
	mat3 tangentToWorldMatrix;
#if defined(TWOSIDED)
	if(gl_FrontFacing)
	{
		tangentToWorldMatrix = mat3(-var_Tangent.xyz, -var_Binormal.xyz, -var_Normal.xyz);
	}
	else
#endif
	{
		tangentToWorldMatrix = mat3(var_Tangent.xyz, var_Binormal.xyz, var_Normal.xyz);
	}


	vec2 texNormal = var_TexNormal.st;
	vec2 texSpecular = var_TexSpecular.st;
	
#if defined(USE_PARALLAX_MAPPING)
	
	// ray intersect in view direction
	
	mat3 worldToTangentMatrix;
	#if defined(GLHW_ATI) || defined(GLHW_ATI_DX10)
	worldToTangentMatrix = mat3(tangentToWorldMatrix[0][0], tangentToWorldMatrix[1][0], tangentToWorldMatrix[2][0],
								tangentToWorldMatrix[0][1], tangentToWorldMatrix[1][1], tangentToWorldMatrix[2][1], 
								tangentToWorldMatrix[0][2], tangentToWorldMatrix[1][2], tangentToWorldMatrix[2][2]);
	#else
	worldToTangentMatrix = transpose(tangentToWorldMatrix);
	#endif

	// compute view direction in tangent space
	vec3 I = worldToTangentMatrix * (u_ViewOrigin - var_Position.xyz);
	I = normalize(I);
	
	// size and start position of search in texture space
	vec2 S = I.xy * -u_DepthScale / I.z;
		
	float depth = RayIntersectDisplaceMap(texNormal, S, u_NormalMap);
	
	// compute texcoords offset
	vec2 texOffset = S * depth;
	
	texDiffuse.st += texOffset;
	texNormal.st += texOffset;
	texSpecular.st += texOffset;
#endif // USE_PARALLAX_MAPPING

	// compute view direction in world space
	vec3 V = normalize(u_ViewOrigin - var_Position);

	// compute half angle in world space
	vec3 H = normalize(L + V);

	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, texNormal.st).xyz - 0.5);
	#if defined(r_NormalScale)
	N.z *= r_NormalScale;
	normalize(N);
	#endif

	// transform normal into world space
	N = tangentToWorldMatrix * N;

	
#else // USE_NORMAL_MAPPING

	vec3 N;
#if defined(TWOSIDED)
	if(gl_FrontFacing)
	{
		N = -normalize(var_Normal.xyz);
	}
	else
#endif
	{
		N = normalize(var_Normal.xyz);
	}
		
#endif // USE_NORMAL_MAPPING

	// compute the light term
#if defined(r_WrapAroundLighting)
	float NL = clamp(dot(N, L) + u_LightWrapAround, 0.0, 1.0) / clamp(1.0 + u_LightWrapAround, 0.0, 1.0);
#else
	float NL = clamp(dot(N, L), 0.0, 1.0);
#endif

	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, texDiffuse.st);
	diffuse.rgb *= u_LightColor * NL;

#if defined(USE_NORMAL_MAPPING)
	// compute the specular term
	vec3 specular = texture2D(u_SpecularMap, texSpecular).rgb * u_LightColor * pow(clamp(dot(N, H), 0.0, 1.0), r_SpecularExponent) * r_SpecularScale;
#endif

	// compute attenuation
	vec3 attenuationXY		= texture2D(u_AttenuationMapXY, var_TexAttenXYZ.xy).rgb;
	vec3 attenuationZ		= texture2D(u_AttenuationMapZ, vec2(var_TexAttenXYZ.z, 0)).rgb;
				
	// compute final color
	vec4 color = diffuse;
#if defined(USE_NORMAL_MAPPING)
	color.rgb += specular;
#endif
	color.rgb *= attenuationXY;
	color.rgb *= attenuationZ;
	color.rgb *= u_LightScale;
	color.rgb *= shadow;

	color.r *= var_TexDiffuse.p;
	color.gb *= var_TexNormal.pq;

	gl_FragColor = color;
	
#if 0
#if defined(USE_PARALLAX_MAPPING)
	gl_FragColor = vec4(vec3(1.0, 0.0, 0.0), diffuse.a);
#elif defined(USE_NORMAL_MAPPING)
	gl_FragColor = vec4(vec3(0.0, 0.0, 1.0), diffuse.a);
#else
	gl_FragColor = vec4(vec3(0.0, 1.0, 0.0), diffuse.a);
#endif
#endif
}
