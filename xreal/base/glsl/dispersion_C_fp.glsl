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

uniform samplerCube	u_ColorMap;
uniform vec3		u_ViewOrigin;
uniform vec3		u_EtaRatio;
uniform float		u_FresnelPower;
uniform float		u_FresnelScale;
uniform float		u_FresnelBias;

varying vec3		var_Vertex;
varying vec3		var_Normal;

void	main()
{	
	/// compute incident ray
	vec3 I = normalize(var_Vertex - u_ViewOrigin);
	
	// compute normal
	vec3 N = normalize(var_Normal);
	
	// compute reflection ray
	vec3 R = reflect(I, N);
	
	// compute fresnel term
	float fresnel = u_FresnelBias + pow(1.0 - dot(I, N), u_FresnelPower) * u_FresnelScale;
	
	// compute reflection color
	vec3 reflectColor = textureCube(u_ColorMap, R).rgb;
	
	// compute refraction color using a refraction ray for each channel
	vec3 refractColor;
	
	refractColor.r = textureCube(u_ColorMap, refract(I, N, u_EtaRatio.x)).r;
	refractColor.g = textureCube(u_ColorMap, refract(I, N, u_EtaRatio.y)).g;
	refractColor.b = textureCube(u_ColorMap, refract(I, N, u_EtaRatio.z)).b;
	
	// compute final color
	gl_FragColor.r = (1.0 - fresnel) * refractColor.r + reflectColor.r * fresnel;
	gl_FragColor.g = (1.0 - fresnel) * refractColor.g + reflectColor.g * fresnel;
	gl_FragColor.b = (1.0 - fresnel) * refractColor.b + reflectColor.b * fresnel;
	gl_FragColor.a = 1.0;
}
