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

uniform mat4		u_LightAttenuationMatrix;
uniform mat4		u_ShadowMatrix;
uniform mat4		u_ModelMatrix;

varying vec4		var_Vertex;
varying vec4		var_TexAtten;
varying vec4		var_TexShadow;

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = ftransform();
	
	// transform position into world space
	var_Vertex.xyz = (u_ModelMatrix * gl_Vertex).xyz;
	
	// calc light attenuation in light space
	var_TexAtten = u_LightAttenuationMatrix * gl_Vertex;
	
	// calc shadow attenuation in light space
	var_TexShadow = u_ShadowMatrix * gl_Vertex;
}
