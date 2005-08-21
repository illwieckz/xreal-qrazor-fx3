/// ============================================================================
/*
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
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

uniform samplerCUBE	u_colormap;
uniform vec3		u_view_origin;

varying vec4		var_vertex;
varying vec3		var_normal;

void	main()
{
	// compute incident ray
	vec3 I = normalize(var_vertex - u_view_origin);
	
	// compute normal
	vec3 N = normalize(var_normal);
	
	// compute reflection ray
	vec3 R = reflect(I, N);
	
	// compute reflection color
	vec3 reflect_color = texCUBE(u_colormap, R).rgb;

	// compute final color
	gl_FragColor = vec4(reflect_color, 1.0);
}
