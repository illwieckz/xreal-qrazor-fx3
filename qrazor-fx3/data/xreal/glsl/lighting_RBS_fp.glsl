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
uniform sampler2D	u_specularmap;
uniform sampler2D	u_lightmap;
uniform sampler2D	u_deluxemap;
uniform vec3		u_view_origin;
uniform float		u_bump_scale;
uniform float		u_specular_exponent;

varying vec3		var_vertex;
varying vec4		var_tex_diffuse_bump;
varying vec2		var_tex_specular;
varying vec2		var_tex_light;
varying vec2		var_tex_deluxe;
varying mat3		var_mat_os2ts;

void	main()
{
	// compute view direction in tangent space
	vec3 V = normalize(var_mat_os2ts * (u_view_origin - var_vertex));

	// compute light direction in tangent space from deluxemap
	vec3 L = normalize(var_mat_os2ts * (2 * (texture2D(u_deluxemap, var_tex_deluxe).xyz - 0.5)));
	
	// compute half angle in tangent space
	vec3 H = normalize(L + V);
	
	// compute normal in tangent space from bumpmap
	vec3 N = 2 * (texture2D(u_bumpmap, var_tex_diffuse_bump.pq).xyz - 0.5);
	N.z *= u_bump_scale;
	N = normalize(N);
	
	// compute light color from object space lightmap
	vec3 C = texture2D(u_lightmap, var_tex_light).xyz;
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_diffusemap, var_tex_diffuse_bump.st);
	diffuse.rgb *= C * saturate(dot(N, L));
	
	// compute the specular term
	vec3 specular = texture2D(u_specularmap, var_tex_specular).rgb * C * pow(saturate(dot(N, H)), u_specular_exponent);
					
	// compute final color
	gl_FragColor = diffuse;
	gl_FragColor.rgb += specular;
}
