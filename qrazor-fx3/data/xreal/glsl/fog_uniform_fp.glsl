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

uniform sampler2D	u_currentrendermap;
//uniform sampler2D	u_heathazemap;
uniform vec2		u_fbuf_scale;
uniform vec2		u_npot_scale;
uniform float		u_fog_density;
uniform vec4		u_fog_color;

varying vec3		var_vertex_view;

void	main()
{
	// calculate fog distance
	float fog_distance = length(var_vertex_view);
	
	// calculate fog exponent
	float fog_exponent = fog_distance * u_fog_density;
	
	// calculate fog factor
	float fog_factor = exp2(-abs(fog_exponent));

	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 s_coord = gl_FragCoord.st;
	s_coord *= u_fbuf_scale;
	clamp(s_coord, 0.0, 1.0);
	
	// scale by the screen non-power-of-two-adjust
	s_coord *= u_npot_scale;

	// get current color
	vec3 current_color = texture2D(u_currentrendermap, s_coord).rgb;
	
	// compute final color, lerp between fog color and current color by fog factor
	gl_FragColor.r = (1.0 - fog_factor) * u_fog_color.r + current_color.r * fog_factor;
	gl_FragColor.g = (1.0 - fog_factor) * u_fog_color.g + current_color.g * fog_factor;
	gl_FragColor.b = (1.0 - fog_factor) * u_fog_color.b + current_color.b * fog_factor;
	gl_FragColor.a = u_fog_color.a;
}
