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

uniform float		u_deform_magnitude;

varying vec2		var_tex_heathaze;
varying float		var_deform;

void	main()
{
	// transform vertex position into homogenous clip-space
	gl_Position = ftransform();
	
	// transform texcoords into heathazemap texture space
	var_tex_heathaze = (gl_TextureMatrix[1] * attr_TexCoord0).st;
	
	// take the deform magnitude and scale it by the projection distance
	vec4 R0 = vec4(1, 0, 0, 1);
	R0.z = dot(gl_ModelViewMatrix[2], gl_Vertex);
	float R1 = dot(gl_ProjectionMatrix[0],  R0);
	float R2 = dot(gl_ProjectionMatrix[3],  R0);
	
	// don't let the recip get near zero for polygons that cross the view plane
	R2 = max(R2, 1.0);
	R2 = 1.0 / R2;
	R1 *= R2;
	
	// clamp the distance so the the deformations don't get too wacky near the view
	R1 = min(R1, 0.02);
	
	var_deform = R1 * u_deform_magnitude;
	
	
}
