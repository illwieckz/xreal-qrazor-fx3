/*
===========================================================================
Copyright (C) 2008-2009 Robert Beckebans <trebor_7@users.sourceforge.net>

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

attribute vec4		attr_Position;
attribute vec4		attr_TexCoord0;
attribute vec3		attr_Tangent;
attribute vec3		attr_Binormal;
attribute vec3		attr_Normal;
//attribute vec4		attr_Color;
attribute vec4		attr_LightColor;
attribute vec3		attr_LightDirection;

uniform mat4		u_DiffuseTextureMatrix;
uniform mat4		u_NormalTextureMatrix;
uniform mat4		u_SpecularTextureMatrix;
uniform int			u_InverseVertexColor;
uniform mat4		u_ModelViewProjectionMatrix;

varying vec3		var_Vertex;
varying vec4		var_TexDiffuseNormal;
varying vec2		var_TexSpecular;
//varying vec4		var_Color;
varying vec4		var_LightColor;
varying vec3		var_LightDirection;
varying vec3		var_Tangent;
varying vec3		var_Binormal;
varying vec3		var_Normal;

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = u_ModelViewProjectionMatrix * attr_Position;
	
	// assign position in object space
	var_Vertex = attr_Position.xyz;
	
	// transform diffusemap texcoords
	var_TexDiffuseNormal.st = (u_DiffuseTextureMatrix * attr_TexCoord0).st;
	
	// transform normalmap texcoords
	var_TexDiffuseNormal.pq = (u_NormalTextureMatrix * attr_TexCoord0).st;
	
	// transform specularmap texture coords
	var_TexSpecular = (u_SpecularTextureMatrix * attr_TexCoord0).st;
	
	// assign light color
	var_LightColor = attr_LightColor;
	
	// assign vertex to light vector in object space
	var_LightDirection = attr_LightDirection;
	
	/*
	// assign color
	if(bool(u_InverseVertexColor))
	{
		var_Color.r = 1.0 - attr_Color.r;
		var_Color.g = 1.0 - attr_Color.g;
		var_Color.b = 1.0 - attr_Color.b;
		var_Color.a = 1.0 - attr_Color.a;
	}
	else
	{
		var_Color = attr_Color;
	}
	*/
	
	var_Tangent = attr_Tangent;
	var_Binormal = attr_Binormal;
	var_Normal = attr_Normal;
}
