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
uniform sampler2D	u_attenuationmap_xy;
uniform sampler2D	u_shadowmap;
uniform vec3		u_light_origin;
uniform vec3		u_light_color;

varying vec3		var_vertex;
varying vec3		var_normal;
varying vec2		var_tex_diffuse;
varying vec4		var_tex_atten_xy_z;
varying vec4		var_tex_shadow;

void	main()
{		
	// compute normal
	vec3 N = normalize(var_normal);
		
	// compute lightdir
	vec3 L = normalize(u_light_origin - var_vertex);
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_diffusemap, var_tex_diffuse);
	diffuse.rgb *= u_light_color * clamp(dot(N, L), 0.0, 1.0);
	
	// compute attenuation
	vec3 attenuation_xy = var_tex_atten_xy_z.w < 0.0 ? vec3(0.0, 0.0, 0.0) : texture2DProj(u_attenuationmap_xy, var_tex_atten_xy_z.xyw).rgb;
	
	// compute shadow
	vec3 shadow = shadow2DProj(u_shadowmap, var_tex_shadow).rgb;
//	vec3 shadow = var_tex_shadow.w < 0.0 ? vec3(0.0, 0.0, 0.0) : texture2DProj(u_shadowmap, var_tex_shadow).rgb;
//	vec3 shadow = var_tex_shadow.w < 0.0 ? vec3(0.0, 0.0, 0.0) : texture2DProj(u_attenuationmap_xy, var_tex_shadow.xyw).rgb;
	
	// compute final color
	gl_FragColor.a = diffuse.a;
	gl_FragColor.rgb = vec3(1.0, 1.0, 1.0);
	gl_FragColor.rgb *= attenuation_xy;
//	gl_FragColor.rgb *= shadow;
}
