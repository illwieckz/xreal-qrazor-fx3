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

varying vec3		var_vertex;
varying vec3		var_normal;

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = ftransform();
	
	// transform position into world space
	var_vertex = (gl_TextureMatrix[0] * gl_Vertex).xyz;
	
	// transform normal into world space
	var_normal.x = dot(gl_TextureMatrix[0][0].xyz, gl_Normal);
	var_normal.y = dot(gl_TextureMatrix[0][1].xyz, gl_Normal);
	var_normal.z = dot(gl_TextureMatrix[0][2].xyz, gl_Normal);
}

