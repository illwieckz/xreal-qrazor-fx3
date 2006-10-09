/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
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

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL.h>

#include "qbsp.h"

// can't use the glvertex3fv functions, because the vec3_t fields
// could be either floats or doubles, depending on DOUBLEVEC_T

qboolean        drawFlag;
static vec3_t   drawOrigin = { 0, 0, 0 };
static vec3_t   drawAngles = { 0, 0, 0 };
static SDL_VideoInfo *drawVideo = NULL;
static SDL_Surface *drawScreen = NULL;


#define	WIN_SIZE	512


static void Reshape(int width, int height)
{
	float           screenaspect;

	// set up viewport
	glViewport(0, 0, width, height);

	// set up projection matrix
	screenaspect = (float)width / (float)height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90, screenaspect, 2, 16384);

	// set up modelview matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1, 0, 0);	// put Z going up
	glRotatef(90, 0, 0, 1);		// put Z going up
	glRotatef(-drawAngles[2], 1, 0, 0);
	glRotatef(-drawAngles[0], 0, 1, 0);
	glRotatef(-drawAngles[1], 0, 0, 1);
	glTranslatef(-drawOrigin[0], -drawOrigin[1], -drawOrigin[2]);
}

static void InitWindow(void)
{
	SDL_Init(SDL_INIT_VIDEO);

	drawVideo = SDL_GetVideoInfo();
	if(!drawVideo)
	{
		Error("Couldn't get video information: %s\n", SDL_GetError());
	}

	// Set the minimum requirements for the OpenGL window
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	/* Note the SDL_DOUBLEBUF flag is not required to enable double 
	 * buffering when setting an OpenGL video mode. 
	 * Double buffering is enabled or disabled using the 
	 * SDL_GL_DOUBLEBUFFER attribute.
	 */

	drawScreen = SDL_SetVideoMode(WIN_SIZE, WIN_SIZE, drawVideo->vfmt->BitsPerPixel, SDL_OPENGL | SDL_RESIZABLE);
	if(!drawScreen)
	{
		SDL_Quit();
		Error("Couldn't set GL video mode: %s\n", SDL_GetError());
	}

	SDL_WM_SetCaption("XMap", "xmap");

	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
}

void DrawBeginScene()
{
	int             w, h, g;
	vec_t           mx, my;
	static qboolean init = qfalse;

	if(!drawFlag)
		return;

	if(!init)
	{
		init = qtrue;
		InitWindow();
	}

	Reshape(drawScreen->w, drawScreen->h);

	glClearColor(1, 0.8, 0.8, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/*
	   w = (drawMaxs[0] - drawMins[0]);
	   h = (drawMaxs[1] - drawMins[1]);

	   mx = drawMins[0] + w / 2;
	   my = drawMins[1] + h / 2;

	   g = w > h ? w : h;

	   glLoadIdentity();
	   gluPerspective(90, 1, 2, 16384);
	   gluLookAt(mx, my, drawMaxs[2] + g / 2, mx, my, drawMaxs[2], 0, 1, 0);
	 */

	glColor3f(0, 0, 0);
//  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if 0
	glColor4f(1, 0, 0, 0.5);
	glBegin(GL_POLYGON);

	glVertex3f(0, 500, 0);
	glVertex3f(0, 900, 0);
	glVertex3f(0, 900, 100);
	glVertex3f(0, 500, 100);

	glEnd();
#endif

	glFlush();
}

void DrawSetRed(void)
{
	if(!drawFlag)
		return;

	glColor3f(1, 0, 0);
}

void DrawSetGrey(void)
{
	if(!drawFlag)
		return;

	glColor3f(0.5, 0.5, 0.5);
}

void DrawSetBlack(void)
{
	if(!drawFlag)
		return;

	glColor3f(0, 0, 0);
}

void DrawWinding(winding_t * w)
{
	int             i;

	if(!drawFlag)
		return;

	glColor4f(0, 0, 0, 0.5);
	glBegin(GL_LINE_LOOP);
	for(i = 0; i < w->numpoints; i++)
		glVertex3f(w->p[i][0], w->p[i][1], w->p[i][2]);
	glEnd();

	glColor4f(0, 0, 1, 0.3);
	glBegin(GL_POLYGON);
	for(i = 0; i < w->numpoints; i++)
		glVertex3f(w->p[i][0], w->p[i][1], w->p[i][2]);
	glEnd();

	glFlush();
}

void DrawAuxWinding(winding_t * w)
{
	int             i;

	if(!drawFlag)
		return;

	glColor4f(0, 0, 0, 0.5);
	glBegin(GL_LINE_LOOP);
	for(i = 0; i < w->numpoints; i++)
		glVertex3f(w->p[i][0], w->p[i][1], w->p[i][2]);
	glEnd();

	glColor4f(1, 0, 0, 0.3);
	glBegin(GL_POLYGON);
	for(i = 0; i < w->numpoints; i++)
		glVertex3f(w->p[i][0], w->p[i][1], w->p[i][2]);
	glEnd();

	glFlush();
}

void DrawPortal(portal_t * p)
{
	winding_t      *w;
	int             sides;

	sides = PortalVisibleSides(p);
	if(!sides)
		return;

	w = p->winding;

	if(sides == 2)				// back side
		w = ReverseWinding(w);

	if(p->areaportal)
	{
		DrawAuxWinding(w);
	}
	else
	{
		DrawWinding(w);
	}

	if(sides == 2)
		FreeWinding(w);
}

static void DrawTree_r(node_t * node)
{
	portal_t       *p, *nextp;

	if(node->planenum != PLANENUM_LEAF)
	{
		DrawTree_r(node->children[0]);
		DrawTree_r(node->children[1]);
		return;
	}

	// draw all the portals
	for(p = node->portals; p; p = nextp)
	{
		if(p->nodes[0] == node)
		{
			DrawPortal(p);

			nextp = p->next[0];
		}
		else
		{
			nextp = p->next[1];
		}
	}
}

void DrawTree(tree_t * tree)
{
	Uint8          *keys;
	qboolean        done;
	matrix_t        rotation;
	vec3_t          forward, right, up;
	qboolean        mouseGrabbed;
	int             oldTime, newTime, deltaTime;	// for frame independent movement

	done = qfalse;
	mouseGrabbed = qfalse;

	oldTime = SDL_GetTicks();
	while(!done)
	{
		SDL_Event       event;
		
		newTime = SDL_GetTicks();
		deltaTime = newTime - oldTime;

		MatrixFromAngles(rotation, drawAngles[PITCH], drawAngles[YAW], drawAngles[ROLL]);
		MatrixToVectorsFRU(rotation, forward, right, up);

		while(SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_VIDEORESIZE:
				{
					drawScreen =
						SDL_SetVideoMode(event.resize.w, event.resize.h, drawVideo->vfmt->BitsPerPixel,
										 SDL_OPENGL | SDL_RESIZABLE);
					if(drawScreen)
					{
						Reshape(drawScreen->w, drawScreen->h);
					}
					else
					{
						/* Uh oh, we couldn't set the new video mode?? */ ;
					}
					break;
				}

				case SDL_MOUSEMOTION:
				{
					if(mouseGrabbed)
					{
						drawAngles[PITCH] += event.motion.yrel;
						drawAngles[YAW] -= event.motion.xrel;
					}
					break;
				}

				case SDL_MOUSEBUTTONDOWN:
				{
					switch (event.button.button)
					{
						case 3:
						{		// K_MOUSE2;
							if(!mouseGrabbed)
							{
								SDL_WM_GrabInput(SDL_GRAB_ON);
								SDL_ShowCursor(0);
								mouseGrabbed = qtrue;
							}
							else
							{
								SDL_ShowCursor(1);
								SDL_WM_GrabInput(SDL_GRAB_OFF);
								mouseGrabbed = qfalse;
							}
							break;
						}

						default:
							break;
					}
					break;
				}

				case SDL_QUIT:
				{
					done = qtrue;
					break;
				}

				default:
					break;
			}
		}


		keys = SDL_GetKeyState(NULL);

		if(keys[SDLK_ESCAPE])
		{
			done = 1;
		}

		if(keys[SDLK_w])
		{
			if(SDL_GetModState() & KMOD_SHIFT)
			{
				VectorMA(drawOrigin, 0.5 * deltaTime, forward, drawOrigin);
			}
			else
			{
				VectorMA(drawOrigin, 1.0 * deltaTime, forward, drawOrigin);
			}
		}

		if(keys[SDLK_s])
		{
			if(SDL_GetModState() & KMOD_SHIFT)
			{
				VectorMA(drawOrigin, -0.5 * deltaTime, forward, drawOrigin);
			}
			else
			{
				VectorMA(drawOrigin, -1.0 * deltaTime, forward, drawOrigin);
			}
		}

		if(keys[SDLK_a])
		{
			if(SDL_GetModState() & KMOD_SHIFT)
			{
				VectorMA(drawOrigin, -0.5 * deltaTime, right, drawOrigin);
			}
			else
			{
				VectorMA(drawOrigin, -1.0 * deltaTime, right, drawOrigin);
			}
		}

		if(keys[SDLK_d])
		{
			if(SDL_GetModState() & KMOD_SHIFT)
			{
				VectorMA(drawOrigin, 0.5 * deltaTime, right, drawOrigin);
			}
			else
			{
				VectorMA(drawOrigin, 1.0 * deltaTime, right, drawOrigin);
			}
		}

		if(keys[SDLK_SPACE])
		{
			//drawOrigin[2] += 1.0 * deltaTime;
			if(SDL_GetModState() & KMOD_SHIFT)
			{
				VectorMA(drawOrigin, 0.5 * deltaTime, up, drawOrigin);
			}
			else
			{
				VectorMA(drawOrigin, 1.0 * deltaTime, up, drawOrigin);
			}
		}

		if(keys[SDLK_c])
		{
			//drawOrigin[2] -= 1.0 * deltaTime;
			if(SDL_GetModState() & KMOD_SHIFT)
			{
				VectorMA(drawOrigin, -0.5 * deltaTime, up, drawOrigin);
			}
			else
			{
				VectorMA(drawOrigin, -1.0 * deltaTime, up, drawOrigin);
			}
		}

		if(keys[SDLK_UP])
		{
			drawAngles[PITCH] -= 1.0 * deltaTime;
		}

		if(keys[SDLK_DOWN])
		{
			drawAngles[PITCH] += 1.0 * deltaTime;
		}

		if(keys[SDLK_LEFT])
		{
			drawAngles[YAW] += 1.0 * deltaTime;
		}

		if(keys[SDLK_RIGHT])
		{
			drawAngles[YAW] -= 1.0 * deltaTime;
		}

		// check to make sure the angles haven't wrapped
		if(drawAngles[PITCH] < -90)
		{
			drawAngles[PITCH] = -90;
		}
		else if(drawAngles[PITCH] > 90)
		{
			drawAngles[PITCH] = 90;
		}

		DrawBeginScene();
		DrawTree_r(tree->headnode);
		DrawEndScene();
		
		oldTime = newTime;
	}

	SDL_Quit();
}

void DrawEndScene(void)
{
	SDL_GL_SwapBuffers();
}
