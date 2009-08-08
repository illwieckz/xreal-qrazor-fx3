/*
===========================================================================
Copyright (C) 2009 Robert Beckebans <trebor_7@users.sourceforge.net>

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
uniform sampler2D	u_DepthMap;
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;
uniform sampler2D	u_ShadowMap;
uniform vec3		u_ViewOrigin;
uniform vec3		u_LightDir;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float       u_LightScale;
uniform mat4		u_LightAttenuationMatrix;
#if !defined(GLHW_ATI) && !defined(GLHW_ATI_DX10)
uniform vec4		u_LightFrustum[6];
#endif
uniform mat4		u_ShadowMatrix;
uniform int			u_ShadowCompare;
uniform int         u_PortalClipping;
uniform vec4		u_PortalPlane;
uniform mat4		u_UnprojectMatrix;

void	main()
{
	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 st = gl_FragCoord.st * r_FBufScale;
	
	// scale by the screen non-power-of-two-adjust
	st *= r_NPOTScale;
		
	// reconstruct vertex position in world space
	float depth = texture2D(u_DepthMap, st).r;
	vec4 P = u_UnprojectMatrix * vec4(gl_FragCoord.xy, depth, 1.0);
	P.xyz /= P.w;
	
	if(bool(u_PortalClipping))
	{
		float dist = dot(P.xyz, u_PortalPlane.xyz) - u_PortalPlane.w;
		if(dist < 0.0)
		{
			discard;
			return;
		}
	}
	
	// transform vertex position into light space
	vec4 texAtten			= u_LightAttenuationMatrix * vec4(P.xyz, 1.0);
	if(texAtten.q <= 0.0)
	{
		// point is behind the near clip plane
		discard;
		return;
	}
	
#if !defined(r_DeferredLighting) && !defined(GLHW_ATI) && !defined(GLHW_ATI_DX10)
	// make sure that the vertex position is inside the light frustum
	for(int i = 0; i < 6; ++i)
	{
		vec4 plane = u_LightFrustum[i];

		float dist = dot(P.xyz, plane.xyz) - plane.w;
		if(dist < 0.0)
		{
			discard;
			return;
		}
	}
#endif

	float shadow = 1.0;
	
#if defined(VSM)
	if(bool(u_ShadowCompare))
	{
		// TODO
	}
	
	if(shadow <= 0.0)
	{
		discard;
	}
	else
#elif defined(ESM)
	if(bool(u_ShadowCompare))
	{
		// TODO
	}
	
	if(shadow <= 0.0)
	{
		discard;
	}
	else
#endif
	{
	
#if !defined(r_DeferredLighting)
		// compute the diffuse term
		vec4 diffuse = texture2D(u_DiffuseMap, st);
#endif

		// compute normal in world space
		vec3 N = 2.0 * (texture2D(u_NormalMap, st).xyz - 0.5);
	
		// compute light direction in world space
		vec3 L = u_LightDir;
	
#if defined(r_NormalMapping)
		// compute view direction in world space
		vec3 V = normalize(u_ViewOrigin - P.xyz);
	
		// compute half angle in world space
		vec3 H = normalize(L + V);
		
#if !defined(r_DeferredLighting)
		vec4 S = texture2D(u_SpecularMap, st);
#endif

#endif // r_NormalMapping
	
		// compute attenuation
		vec3 attenuationXY = texture2DProj(u_AttenuationMapXY, texAtten.xyw).rgb;
		vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(0.5 + texAtten.z, 0.0)).rgb; // FIXME
	
		// compute final color
#if defined(r_DeferredLighting)
		vec4 color = vec4(1.0, 1.0, 1.0, 1.0);
#else
		vec4 color = diffuse;
#endif
		
		color.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
		
#if defined(r_NormalMapping)

#if defined(r_DeferredLighting)
		//color.a += pow(clamp(dot(N, H), 0.0, 1.0), r_SpecularExponent) * r_SpecularScale;
#else
		color.rgb += S.rgb * u_LightColor * pow(clamp(dot(N, H), 0.0, 1.0), r_SpecularExponent) * r_SpecularScale;
#endif

#endif // r_NormalMapping

		//color.rgb *= attenuationXY;
		//color.rgb *= attenuationZ;
		color.rgb *= u_LightScale;
		color.rgb *= shadow;
		
		gl_FragColor = color;
	}
}
