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

uniform sampler2D	u_cloudmap;

void	main()
{
//	vec4 horizon = texture2D(s_texture_0, gl_TexCoord[0].xy);
//	vec4 clouds_0 = texture2D(s_texture_1, gl_TexCoord[1].xy);
//	vec4 clouds_1 = texture2D(s_texture_1, gl_TexCoord[2].xy);
	
//	vec4 clouds = (clouds_0 + clouds_1) * horizon.w;
	
//	gl_FragColor = horizon * (1.0 - clouds.x) + clouds;

	gl_FragColor = texture2D(u_cloudmap, gl_TexCoord[0].xy);
//	gl_FragColor *= u_color;
}
