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

uniform sampler2D	u_diffusemap;

varying vec3		var_normal;
varying vec3		var_light;
varying vec2		var_tex_diffuse;
varying vec3		var_color;

void	main()
{		
	// compute normal
	vec3 N = normalize(var_normal);
		
	// compute lightdir
	vec3 L = normalize(var_light);
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_diffusemap, var_tex_diffuse);
	diffuse.rgb *= var_color * clamp(dot(N, L), 0.0, 1.0);
	
	// compute final color
	gl_FragColor.rgba = diffuse;
}
