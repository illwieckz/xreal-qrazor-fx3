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

attribute vec4		attr_TexCoord0;
attribute vec3		attr_Tangent;
attribute vec3		attr_Binormal;

uniform int			u_InverseVertexColor;
uniform mat4		u_LightAttenuationMatrix;
uniform mat4		u_ModelMatrix;

varying vec3		var_Vertex;
varying vec4		var_TexDiffuse;
varying vec4		var_TexNormal;
varying vec2		var_TexSpecular;
varying vec3		var_TexAttenXYZ;
varying mat3		var_TangentToWorldMatrix;
//varying vec4		var_Color;	// Tr3B - maximum vars reached

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = ftransform();
	
	// transform position into world space
	var_Vertex = (u_ModelMatrix * gl_Vertex).xyz;
		
	// transform diffusemap texcoords
	var_TexDiffuse.xy = (gl_TextureMatrix[0] * attr_TexCoord0).st;
	
	// transform normalmap texcoords
	var_TexNormal.xy = (gl_TextureMatrix[1] * attr_TexCoord0).st;
	
	// transform specularmap texture coords
	var_TexSpecular = (gl_TextureMatrix[2] * attr_TexCoord0).st;
	
	// calc light xy,z attenuation in light space
	var_TexAttenXYZ = (u_LightAttenuationMatrix * gl_Vertex).xyz;
	
	// construct tangent-space-to-world-space 3x3 matrix
	vec3 tangent = (u_ModelMatrix * vec4(attr_Tangent, 0.0)).xyz;
	vec3 binormal = (u_ModelMatrix * vec4(attr_Binormal, 0.0)).xyz;
	vec3 normal = (u_ModelMatrix * vec4(gl_Normal, 0.0)).xyz;
	
	var_TangentToWorldMatrix = mat3(tangent, binormal, normal);
	
	// assign color
	if(bool(u_InverseVertexColor))
	{
		var_TexDiffuse.p = 1.0 - gl_Color.r;
		var_TexNormal.p = 1.0 - gl_Color.g;
		var_TexNormal.q = 1.0 - gl_Color.b;
	}
	else
	{
		var_TexDiffuse.p = gl_Color.r;
		var_TexNormal.pq = gl_Color.gb;
	}
}
