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

attribute vec4		attr_TexCoord0;

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = ftransform();
	
	// transform texcoords
	gl_TexCoord[0] = gl_TextureMatrix[0] * attr_TexCoord0;
	
//	gl_TexCoord[1] = vec4(0.0, 1.0 - gl_Vertex.z / 600.0 + 0.3, 0.0, 1.0);
//	gl_TexCoord[2] = vec4(gl_Vertex.x / 2000.0 + u_time / 80.0, gl_Vertex.y / 2000.0, 0.0, 1.0);
//	gl_TexCoord[3] = vec4(gl_Vertex.x / 1000.0 + u_time / 90.0, gl_Vertex.y / 1000.0, 0.0, 1.0);
}
