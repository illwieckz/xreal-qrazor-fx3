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

varying vec4		var_Color;

void	main()
{
#if defined(GL_ARB_draw_buffers)
	gl_FragData[0] = var_Color;
	vec4 black = vec4(0.0, 0.0, 0.0, 1.0);
	gl_FragData[1] = black;
	gl_FragData[2] = black;
	gl_FragData[3] = black;
#else
	gl_FragColor = var_Color;
#endif
}
