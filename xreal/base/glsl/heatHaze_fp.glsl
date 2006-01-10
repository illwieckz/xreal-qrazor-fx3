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

uniform sampler2D	u_ColorMap;
uniform sampler2D	u_NormalMap;
uniform vec2		u_FBufScale;
uniform vec2		u_NPotScale;

varying vec2		var_TexNormal;
varying float		var_Deform;

void	main()
{
	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal).agb - 0.5);
	N = normalize(N);

	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 s_coord = gl_FragCoord.st * u_FBufScale;
	
	// offset by the scaled normal and clamp it to 0.0 - 1.0
	s_coord += N.xy * var_Deform;
	clamp(s_coord, 0.0, 1.0);
	
	// scale by the screen non-power-of-two-adjust
	s_coord *= u_NPotScale;

	gl_FragColor = texture2D(u_ColorMap, s_coord);
}
