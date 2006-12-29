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
uniform float		u_SpecularExponent;

varying vec3		var_Vertex;
varying vec2		var_TexDiffuse;
varying vec2		var_TexNormal;
varying vec2		var_TexSpecular;
varying mat3		var_TS2WSMatrix;

void	main()
{
	vec4 diffuse = texture2D(u_DiffuseMap, var_TexDiffuse);
	vec3 specular = texture2D(u_SpecularMap, var_TexSpecular).rgb;

	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal).xyz - 0.5);
	
	// transform normal into world space
	N = var_TS2WSMatrix * N;
	
	// convert normal back to [0,1] color space
	N = N * 0.5 + 0.5;

	gl_FragData[0] = diffuse;
	gl_FragData[1] = vec4(N, 0.0);
	gl_FragData[2] = vec4(specular, u_SpecularExponent);
	gl_FragData[3] = vec4(var_Vertex, 1.0);
}
