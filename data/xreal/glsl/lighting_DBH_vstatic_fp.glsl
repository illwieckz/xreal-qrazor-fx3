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

uniform sampler2D	u_diffusemap;
uniform sampler2D	u_bumpmap;
uniform vec3		u_view_origin;
uniform float		u_bump_scale;
uniform float		u_height_scale;
uniform float		u_height_bias;

varying vec3		var_vertex;
varying vec2		var_tex_diffuse;
varying vec2		var_tex_bump;
varying mat3		var_mat_os2ts;
varying vec3		var_light;
varying vec3		var_color;

void	main()
{	
	// compute view direction in tangent space
	vec3 V = normalize(var_mat_os2ts * (u_view_origin - var_vertex));
	
	// compute height
	float height = texture2D(u_bumpmap, var_tex_bump).a;
	
	// compute texcoords offset
	vec2 tex_offset = (height * u_height_scale + u_height_bias) * V.xy;
		
	// compute light direction in tangent space
	vec3 L = normalize(var_mat_os2ts * var_light);
	
	// compute normal in tangent space from bumpmap
	vec3 N = 2.0 * (texture2D(u_bumpmap, var_tex_bump + tex_offset).xyz - 0.5);
	N.z *= u_bump_scale;
	N = normalize(N);
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_diffusemap, var_tex_diffuse + tex_offset);
	diffuse.rgb *= var_color * clamp(dot(N, L), 0.0, 1.0);
					
	// compute final color
	gl_FragColor.rgba = diffuse;
}

