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

uniform sampler2D	u_currentrendermap;
uniform sampler2D	u_heathazemap;
uniform vec2		u_fbuf_scale;
uniform vec2		u_npot_scale;
uniform float		u_bump_scale;

varying vec2		var_tex_heathaze;

void	main()
{
	// compute normal in tangent space from bumpmap
	vec3 N = 2 * (texture2D(u_heathazemap, var_tex_heathaze).xyz - 0.5);
	N.z *= u_bump_scale;
	N = normalize(N);

	vec2 s_coord = gl_FragCoord.xy;

	// calculate the screen texcoord in the 0.0 to 1.0 range
	s_coord *= u_fbuf_scale;
	
	// offset by the scaled normal and clamp it to 0.0 - 1.0
	s_coord += N.xy;
	clamp(s_coord, 0.0, 1.0);
	
	// scale by the screen non-power-of-two-adjust
	s_coord *= u_npot_scale;

	gl_FragColor = texture2D(u_currentrendermap, s_coord);
//	gl_FragColor *= vec4(0.5, 0.5, 0.9, 1.0);
}
