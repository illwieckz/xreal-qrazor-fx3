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

uniform	sampler2D	u_diffusemap;
uniform sampler2D	u_bumpmap;
uniform sampler2D	u_lightmap;
uniform sampler2D	u_deluxemap;
uniform float		u_bump_scale;

varying vec2		var_tex_diffuse;
varying vec2		var_tex_bump;
varying vec2		var_tex_light;
varying vec2		var_tex_deluxe;
varying mat3		var_mat_os2ts;

void	main()
{	
	// compute light direction in tangent space from deluxemap
	vec3 L = normalize(var_mat_os2ts * (2 * (texture2D(u_deluxemap, var_tex_deluxe).xyz - 0.5)));
	
	// compute normal in tangent space from bumpmap
	vec3 N = 2 * (texture2D(u_bumpmap, var_tex_bump).xyz - 0.5);
	N.z *= u_bump_scale;
	N = normalize(N);
	
	// compute light color from object space lightmap
	vec3 C = texture2D(u_lightmap, var_tex_light).xyz;
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_diffusemap, var_tex_diffuse);
	diffuse.rgb *= C * saturate(dot(N, L));
					
	// compute final color
	gl_FragColor = diffuse;
}
