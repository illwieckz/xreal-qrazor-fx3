/// ============================================================================
/*
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
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

uniform vec3		u_LightOrigin;

varying vec4		var_Color;

void	main()
{
	if(gl_Vertex.w == 1.0)
	{
		// transform vertex position into homogenous clip-space
		gl_Position = ftransform();
	}
	else
	{
		// project vertex position to infinity
		vec4 vertex = vec4((gl_Vertex.xyz - u_LightOrigin), 0.0);
		gl_Position	= gl_ModelViewProjectionMatrix * vertex;
	}
	
	// assign color
	var_Color = gl_Color;
}
