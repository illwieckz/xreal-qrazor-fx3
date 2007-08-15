/*
===========================================================================
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

uniform sampler2D	u_ColorMap;
uniform float		u_AlphaTest;
uniform vec3		u_LightOrigin;
uniform float       u_LightRadius;

varying vec3		var_Vertex;
varying vec2		var_Tex;
varying vec4		var_Color;

void	main()
{
	vec4 color = texture2D(u_ColorMap, var_Tex);

	if(color.a <= u_AlphaTest)
	{
		discard;
	}
	
	float distance = length(var_Vertex - u_LightOrigin) / u_LightRadius;
	float distanceSquared = distance * distance;

#if defined(VSM_CLAMP)
	// convert to [0,1] color space
	gl_FragColor = vec4(distance, distanceSquared, 0.0, 0.0) * 2.0 - 1.0;
#else
	gl_FragColor = vec4(distance, distanceSquared, 0.0, 0.0);
#endif
}
