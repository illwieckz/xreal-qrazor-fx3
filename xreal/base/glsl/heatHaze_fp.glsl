/*
===========================================================================
Copyright (C) 2006 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2006 defconx          <defcon-x@ns-co.net>

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

uniform sampler2D	u_NormalMap;
uniform sampler2D	u_CurrentMap;
uniform sampler2D	u_ContrastMap;
uniform vec2		u_FBufScale;
uniform vec2		u_NPOTScale;

varying vec2		var_TexNormal;
varying float		var_Deform;

void	main()
{
	vec4 color0, color1;

	// compute normal in tangent space from normalmap
	vec3 N = 2.0 * (texture2D(u_NormalMap, var_TexNormal).xyz - 0.5);
	N = normalize(N);

	// calculate the screen texcoord in the 0.0 to 1.0 range
	vec2 st = gl_FragCoord.st * u_FBufScale;
	
	// offset by the scaled normal and clamp it to 0.0 - 1.0
	st += N.xy * var_Deform;
	st = clamp(st, 0.0, 1.0);
	
	// scale by the screen non-power-of-two-adjust
	st *= u_NPOTScale;
	
	// check if the distortion got too far
	vec3 vis = texture2D(u_ContrastMap, st).rgb;
	if(length(vis) > 0.0)
	{
		color0 = texture2D(u_CurrentMap, st);
		color1 = vec4(0.0, 1.0, 0.0, color0.a);
	}
	else
	{
		// reset st and don't offset
		st = gl_FragCoord.st * u_FBufScale * u_NPOTScale;
		
		color0 = texture2D(u_CurrentMap, st);
		color1 = vec4(1.0, 0.0, 0.0, color0.a);
	}
	
#if defined(GL_ARB_draw_buffers)
	gl_FragData[0] = color0;
	gl_FragData[1] = color1;
	gl_FragData[2] = vec4(0.0, 0.0, 0.0, color0.a);
	gl_FragData[3] = vec4(0.0, 0.0, 0.0, color0.a);
#else
	gl_FragColor = color0;
#endif
}
