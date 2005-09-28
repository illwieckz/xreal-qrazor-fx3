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

uniform sampler2D	u_DiffuseMap;
uniform sampler2D	u_NormalMap;
uniform sampler2D	u_AttenuationMapXY;
uniform sampler2D	u_AttenuationMapZ;
//uniform samplerCube	u_AttenuationMapCube;
uniform vec3		u_LightOrigin;
uniform vec3		u_LightColor;

varying vec3		var_Vertex;
varying vec2		var_TexDiffuse;
varying vec2		var_TexNormal;
varying vec3		var_TexAttenXYZ;
//varying vec3		var_TexAttenCube;
varying mat3		var_OS2TSMatrix;

void	main()
{
	// compute light direction in tangent space
	vec3 L = normalize(var_OS2TSMatrix * (u_LightOrigin - var_Vertex));
	
	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal).xyz - 0.5);
	N = normalize(N);
	
	// compute the diffuse term
	vec4 diffuse = texture2D(u_DiffuseMap, var_TexDiffuse);
	diffuse.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
	
	// compute attenuation
	vec3 attenuationXY		= texture2D(u_AttenuationMapXY, var_TexAttenXYZ.xy).rgb;
	vec3 attenuationZ		= texture2D(u_AttenuationMapZ, vec2(var_TexAttenXYZ.z, 0)).rgb;
//	vec3 attenuationCube	= textureCube(u_AttenuationMapCube, var_tex_atten_cube).rgb;
					
	// compute final color
//	gl_FragColor.rgba = vec4(1.0, 1.0, 1.0, 1.0);
//	gl_FragColor.rgb *= u_LightColor * clamp(dot(N, L), 0.0, 1.0);
	gl_FragColor.rgba = diffuse;
	gl_FragColor.rgb *= attenuationXY;
	gl_FragColor.rgb *= attenuationZ;
//	gl_FragColor.rgb *= attenuationCube;
}
