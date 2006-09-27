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

uniform samplerCube	u_ColorMap;
uniform sampler2D	u_NormalMap;
uniform vec3		u_ViewOrigin;
uniform mat4		u_ModelMatrix;

varying vec3		var_Vertex;
varying vec2		var_TexNormal;
varying mat3		var_TS2OSMatrix;

void	main()
{
	vec3 I, N, R;
	vec4 reflectColor;

	// compute incident ray in world space
	I = normalize(var_Vertex - u_ViewOrigin);
	
	// compute normal in tangent space from normalmap
	N = 2.0 * (texture2D(u_NormalMap, var_TexNormal).xyz - 0.5);
	
	// transform normal into object space
	N = var_TS2OSMatrix * N;
	
	// transform normal into world space
	N = normalize((u_ModelMatrix * vec4(N, 0.0)).xyz);
	
	// compute reflection ray
	R = reflect(I, N);
	
	// compute reflection color
	reflectColor = textureCube(u_ColorMap, R).rgba;

	// compute final color
	vec4 color = reflectColor;
	
	gl_FragColor = color;
}
