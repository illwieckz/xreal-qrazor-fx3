/// ============================================================================
/*
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
Please see the file "AUTHORS" for a list of contributors

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
/// ============================================================================

uniform samplerCube	u_ColorMap;
uniform vec3		u_ViewOrigin;

varying vec3		var_Vertex;
varying vec3		var_Normal;

void	main()
{
	// compute incident ray
	vec3 I = normalize(var_Vertex - u_ViewOrigin);
	
	// compute normal
	vec3 N = normalize(var_Normal);
	
	// compute refraction ray
	vec3 R = refract(I, N, 1.0);
	
	// compute reflection color
	vec4 color = textureCube(u_ColorMap, R).rgba;

	// compute final color
	gl_FragColor = color;
}
