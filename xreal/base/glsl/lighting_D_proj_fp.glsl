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
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;
uniform sampler2D	u_ShadowMap;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightColor;
uniform float		u_LightRadius;
uniform float		u_LightScale;

varying vec3		var_Vertex;
varying vec3		var_Normal;
varying vec2		var_TexDiffuse;
varying vec4		var_TexAtten;

void	main()
{
	if(var_TexAtten.q <= 0.0)
	{
		discard;
	}

	// compute normal
	vec3 N = normalize(var_Normal);
		
	// compute lightdir
	vec3 L = normalize(u_LightOrigin - var_Vertex);
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, var_TexDiffuse);
	diffuse.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
	
	// compute attenuation	
	vec3 attenuationXY = texture2DProj(u_AttenuationMapXY, var_TexAtten.xyw).rgb;
	vec3 attenuationZ  = texture2D(u_AttenuationMapZ, vec2(var_TexAtten.z, 0.0)).rgb;

	// compute final color
	vec4 color = diffuse;
	color.rgb *= attenuationXY;
//	color.rgb *= attenuationZ;
	color.rgb *= u_LightScale;

#if defined(SHADOWMAPPING)
	float vertexDistance = length(var_Vertex - u_LightOrigin) / u_LightRadius - 0.005f;
	vec4 extract = vec4(1.0, 0.00390625, 0.0000152587890625, 0.000000059604644775390625);
#if 1
	float shadowDistance = dot(texture2DProj(u_ShadowMap, var_TexAtten.xyw), extract);
	float shadow = vertexDistance <= shadowDistance ? 1.0 : 0.0;
	color.rgb *= shadow;
#else
	float shadow = 0.0;
	
	vec2 projCoords = var_TexAtten.st / var_TexAtten.q;

	float shadowDistance = dot(texture2D(u_ShadowMap, projCoords.st + vec2(0.02, 0.02)), extract);
	if(vertexDistance <= shadowDistance)
		shadow = shadow + 1.0;
		
	shadowDistance = dot(texture2D(u_ShadowMap, projCoords.st + vec2(0.02, -0.02)), extract);
	if(vertexDistance <= shadowDistance)
		shadow = shadow + 1.0;
		
	shadowDistance = dot(texture2D(u_ShadowMap, projCoords.st + vec2(-0.02, 0.02)), extract);
	if(vertexDistance <= shadowDistance)
		shadow = shadow + 1.0;
		
	shadowDistance = dot(texture2D(u_ShadowMap, projCoords.st + vec2(-0.02, -0.02)), extract);
	if(vertexDistance <= shadowDistance)
		shadow = shadow + 1.0;

	shadow /= 4.0;
	color.rgb *= shadow;
#endif
#endif
	
	gl_FragColor = color;
}
