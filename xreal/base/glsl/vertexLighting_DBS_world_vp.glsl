/*
===========================================================================
Copyright (C) 2008 Robert Beckebans <trebor_7@users.sourceforge.net>

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

varying vec3		var_Vertex;
varying vec4		var_TexDiffuseNormal;
varying vec2		var_TexSpecular;
varying vec3		var_LightDir;
varying vec4		var_Color;
varying vec3		var_Tangent;
varying vec3		var_Binormal;
varying vec3		var_Normal;

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	
	// assign position in object space
	var_Vertex = gl_Vertex.xyz;
	
	// transform diffusemap texcoords
	var_TexDiffuseNormal.st = (gl_TextureMatrix[0] * attr_TexCoord0).st;
	
	// transform normalmap texcoords
	var_TexDiffuseNormal.pq = (gl_TextureMatrix[1] * attr_TexCoord0).st;
	
	// transform specularmap texture coords
	var_TexSpecular = (gl_TextureMatrix[2] * attr_TexCoord0).st;
	
	// assign vertex to light vector in object space
	//var_light = attr_Light;
	// FIXME
	var_LightDir = gl_Normal;
	
	// assign color
	var_Color = gl_Color;
	
	var_Tangent = attr_Tangent;
	var_Binormal = attr_Binormal;
	var_Normal = gl_Normal;
}
