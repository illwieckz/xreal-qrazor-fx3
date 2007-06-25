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
uniform sampler2D	u_ShadowMap;
uniform vec3		u_ViewOrigin;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float       u_LightScale;
uniform mat4		u_LightAttenuationMatrix;
uniform mat4		u_ShadowMatrix;
uniform int			u_ShadowCompare;
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
	vec4 texAtten			= u_LightAttenuationMatrix * vec4(P.xyz, 1.0);
	vec3 attenuationXY		= texture2DProj(u_AttenuationMapXY, texAtten.xyw).rgb;
	vec3 attenuationZ		= texture2D(u_AttenuationMapZ, vec2(1.0 - texAtten.z, 0.0)).rgb;
	
	// compute final color
	vec4 color = diffuse;
	color.rgb += specular;
	color.rgb *= attenuationXY;
	color.rgb *= attenuationZ;
	color.rgb *= u_LightScale;
	
#if defined(VSM)
	if(bool(u_ShadowCompare))
	{
		float vertexDistance = length(P.xyz - u_LightOrigin) / u_LightRadius;
		
		vec4 texShadow = u_ShadowMatrix * vec4(P.xyz, 1.0);
		vec2 shadowDistances = texture2DProj(u_ShadowMap, texShadow.xyw).rg;
	
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
