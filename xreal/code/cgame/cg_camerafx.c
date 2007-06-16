/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.
Copyright (C) 2007 Jeremy Hughes <Encryption767@msn.com>

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
#include "cg_local.h"

/*
===================
CG_InitCameraFX

This is called at startup and for tournement restarts
===================
*/
void	CG_InitCameraFX( void ) {
	int		i;


	memset( cg_cameraFX, 0, sizeof( cg_cameraFX ) );
	cg_activeCameraFX.next = &cg_activeCameraFX;
	cg_activeCameraFX.prev = &cg_activeCameraFX;
	cg_freeCameraFX = cg_cameraFX;
	for ( i = 0 ; i < MAX_CAMERAFX - 1 ; i++ ) {
		cg_cameraFX[i].next = &cg_cameraFX[i+1];
	}



}


/*
==================
CG_FreeCameraFX
==================
*/
void CG_FreeCameraFX( cameraFX_t *cfx ) {
	if ( !cfx->prev ) {
		CG_Error( "CG_FreeCameraFX: not active" );
	}

	// remove from the doubly linked active list
	cfx->prev->next = cfx->next;
	cfx->next->prev = cfx->prev;

	// the free list is only singly linked
	cfx->next = cg_freeCameraFX;
	cg_freeCameraFX = cfx;
}

/*
===================
CG_AllocCameraFX

Will allways succeed, even if it requires freeing an old active entity
===================
*/
cameraFX_t	*CG_AllocCameraFX( void ) {
	cameraFX_t	*cfx;

	if ( !cg_freeCameraFX ) {
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		if(!(cg_activeCameraFX.prev->cfxFlags & CFXF_NEVER_FREE)){
			CG_FreeCameraFX( cg_activeCameraFX.prev );
		}
	}

	cfx = cg_freeCameraFX;
	cg_freeCameraFX = cg_freeCameraFX->next;

	memset( cfx, 0, sizeof( *cfx ) );

	// link into the active list
	cfx->next = cg_activeCameraFX.next;
	cfx->prev = &cg_activeCameraFX;
	cg_activeCameraFX.next->prev = cfx;
	cg_activeCameraFX.next = cfx;
	return cfx;
}

static void CG_ColorFlare( vec3_t org ,int duration,int radius,vec3_t color) {
	cameraFX_t	*cfx;
	refEntity_t		*re;

	if (duration < 0) {
		duration *= -1;
	}
	cfx = CG_AllocCameraFX();
	re = &cfx->refEntity;

	cfx->cfxType = CFX_CORONA;
	cfx->startTime = cg.time ;
	cfx->endTime = cg.time + duration;
	cfx->cfxFlags = CFXF_DISTANCE_SCALE;
	cfx->lifeRate = 1.0 ;
	cfx->color[0] = color[0];
	cfx->color[1] = color[1];
	cfx->color[2] = color[2];
	cfx->color[3] = 1.0;
	cfx->pos.trType = TR_LINEAR;
	cfx->pos.trTime = cg.time;
	cfx->MaxRadius = radius;
	cfx->MinRadius = radius / 4;
	re->shaderTime = cg.time / 1000.0f;

	re->customShader = cgs.media.colorflare;

	VectorCopy( org, re->origin );
	VectorCopy( org, re->oldorigin );

}

//==============================================================================
void CG_ParseFXFile( const char *filename) {
	char		*text_p, *token, text[9000000];
	int			len, skip, i;
	fileHandle_t	f;

	// load the file
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		return ;
	}
	if ( len >= sizeof( text ) - 1 ) {
		CG_Printf( "File %s too long\n", filename );
		return ;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning
	i = 1;
	while ( 1 ) {

//		prev = text_p;	// so we can unget		

		token = Com_ParseExt( &text_p ,qtrue);

		if (atoi(token) == i){

			token = Com_ParseExt( &text_p ,qtrue);
			if ( !token ) {
				break;
			}

			if ( !Q_stricmp( token, "flare" ) ) {
				vec3_t	origin;
				int		size;
					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[0] = atoi(token);
					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[1] = atoi(token);
					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[2] = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					size = atoi(token);
					CG_RailCorona( origin ,6000000,size);
				i++;
			}else if ( !Q_stricmp( token, "colorflare" ) ) {
				vec3_t	origin, color;
				int		size;
					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[0] = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[1] = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[2] = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					size = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					color[0] = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					color[1] = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					color[2] = atoi(token);
					CG_ColorFlare( origin ,6000000,size,color);
				i++;
			}else if ( !Q_stricmp( token, "fire" ) ) {
				vec3_t	origin;
				int		size,endsize;
					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[0] = atoi(token);
					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[1] = atoi(token);
					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					origin[2] = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					size = atoi(token);

					token = Com_ParseExt( &text_p ,qtrue);
					if ( !token ) {
						break;
					}
					endsize = atoi(token);

				i++;
			}


			continue;
		}else{
			break;
		}
	}
}
/*
qboolean	CG_ParseFXFile( const char *filename ) {
	char		*text_p, *prev;
	int			len;
	char		*token;
	int			skip;
	char		text[20000];
	fileHandle_t	f;

	// load the file
	len = trap_FS_FOpenFile( filename, &f, FS_READ );
	if ( len <= 0 ) {
		return qfalse;
	}
	if ( len >= sizeof( text ) - 1 ) {
		CG_Printf( "File %s too long\n", filename );
		return qfalse;
	}
	trap_FS_Read( text, len, f );
	text[len] = 0;
	trap_FS_FCloseFile( f );

	// parse the text
	text_p = text;
	skip = 0;	// quite the compiler warning

	// read optional parameters
	while ( 1 ) {
		prev = text_p;	// so we can unget
		token = Com_Parse( &text_p );
		if ( !token ) {
			break;
		}

		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}else if ( !Q_stricmp( token, "rlcorona" ) ) {
			vec3_t	origin;
			int		size;
			while (1){
				token = Com_ParseExt( &text_p ,qtrue);
				if ( !token ) {
					return qfalse;
				}
				origin[0] = atoi(token);

				token = Com_ParseExt( &text_p ,qtrue);
				if ( !token ) {
					return qfalse;
				}
				origin[1] = atoi(token);

				token = Com_ParseExt( &text_p ,qtrue);
				if ( !token ) {
					return qfalse;
				}
				origin[2] = atoi(token);

				token = Com_ParseExt( &text_p ,qtrue);
				if ( !token ) {
					return qfalse;
				}
				size = atoi(token);
			}
			CG_RailCorona( origin ,6000000,size);
			continue;
		}

		if ( token[0] >= '0' && token[0] <= '9' ) {
			text_p = prev;	// unget the token
			break;
		}
		Com_Printf( "unknown token '%s' is %s\n", token, filename );
	}
	return qtrue;
}
*/
void CG_RailCorona( vec3_t org ,int duration,int radius) {
	cameraFX_t	*cfx;
	refEntity_t		*re;

	if (duration < 0) {
		duration *= -1;
	}
	cfx = CG_AllocCameraFX();
	re = &cfx->refEntity;

	cfx->cfxType = CFX_CORONA;
	cfx->startTime = cg.time ;
	cfx->endTime = cg.time + duration;
	cfx->cfxFlags = CFXF_DISTANCE_SCALE;
	cfx->lifeRate = 1.0 ;
	cfx->color[0] = cfx->color[1] = cfx->color[2] = cfx->color[3] = 1.0;
	cfx->pos.trType = TR_LINEAR;
	cfx->pos.trTime = cg.time;
	cfx->MaxRadius = radius;
	cfx->MinRadius = radius / 2;
	re->shaderTime = cg.time / 1000.0f;

	re->customShader = cgs.media.railCoronaShader;

	VectorCopy( org, re->origin );
	VectorCopy( org, re->oldorigin );

}

void CG_RocketCorona( vec3_t org ,int duration,int radius) {
	cameraFX_t	*cfx;
	refEntity_t		*re;

	if (duration < 0) {
		duration *= -1;
	}
	cfx = CG_AllocCameraFX();
	re = &cfx->refEntity;

	cfx->cfxType = CFX_CORONA;
	cfx->startTime = cg.time ;
	cfx->endTime = cg.time + duration;
	cfx->cfxFlags = CFXF_DISTANCE_SCALE;
	cfx->lifeRate = 1.0 ;
	cfx->color[0] = cfx->color[1] = cfx->color[2] = cfx->color[3] = 1.0;
	cfx->pos.trType = TR_LINEAR;
	cfx->pos.trTime = cg.time;
	cfx->MaxRadius = radius;
	cfx->MinRadius = radius / 2;

	re->shaderTime = cg.time / 1000.0f;

	re->customShader = cgs.media.rocketCoronaShader;

	VectorCopy( org, re->origin );
	VectorCopy( org, re->oldorigin );

}

/*
==================
CG_Corona
An attempt for creating the do all polygon for camera effects. :) 
==================
*/
static void CG_Corona( cameraFX_t *cfx ) {
	polyVert_t verts[4];
	refEntity_t	*re;
	float		c,len,sizeup,sizedown,dscale;
	vec3_t	p,rotate_ang,rr, ru,delta,rlforward, rlvright, rlvup/*,color*/;
//	int i, j;
//	float		ratio;
//	float		time, time2;
	trace_t trace;

	re = &cfx->refEntity;

	if ( cfx->fadeInTime > cfx->startTime && cg.time < cfx->fadeInTime ) {
		// fade / grow time
		c = 1.0 - (float) ( cfx->fadeInTime - cg.time ) / ( cfx->fadeInTime - cfx->startTime );
	}
	else {
	//	 fade / grow time
		c = ( cfx->endTime - cg.time ) / ( float ) ( cfx->endTime - cfx->startTime );
	//	c = (float) ( cfx->endTime - cg.time ) * cfx->lifeRate;
	}

	if( cfx->cfxFlags & CFXF_NO_RGB_FADE ){ 
		re->shaderRGBA[0] = (unsigned char) 255 * cfx->color[0];
		re->shaderRGBA[1] = (unsigned char) 255 * cfx->color[1];
		re->shaderRGBA[2] = (unsigned char) 255 * cfx->color[2];
	}else{
		re->shaderRGBA[0] = (unsigned char) 255 * c * cfx->color[0];
		re->shaderRGBA[1] = (unsigned char) 255 * c * cfx->color[1];
		re->shaderRGBA[2] = (unsigned char) 255 * c * cfx->color[2];
	}
	if( cfx->cfxFlags & CFXF_NO_ALPHA_FADE ){ 
		re->shaderRGBA[3] = 255;
	}else{
		re->shaderRGBA[3] = (unsigned char) 255 * c * cfx->color[3];
	}

		if (re->rotation) {
			VectorCopy( cg.CameraFXView.viewaxis[0], rlforward );
			VectorCopy( cg.CameraFXView.viewaxis[1], rlvright );
			VectorCopy( cg.CameraFXView.viewaxis[2], rlvup );
			vectoangles(cg.CameraFXView.viewaxis[0], rotate_ang );
			rotate_ang[ROLL] += re->rotation;
			AngleVectors ( rotate_ang, rlforward, rr, ru);
		} else {
			VectorCopy( cg.CameraFXView.viewaxis[0], rlforward );
			VectorCopy( cg.CameraFXView.viewaxis[1], rlvright );
			VectorCopy( cg.CameraFXView.viewaxis[2], rlvup );
		}
	

	// scale up?
	sizeup = cfx->radius * ( 1.0 - c ) + 8;
	// or down 
	sizedown = cfx->radius * c;

	dscale = Distance( cg.refdef[0].vieworg, re->origin ) * 0.1f;
	CG_Trace( &trace, cg.CameraFXView.vieworg, NULL, NULL, re->origin, -1, MASK_FLARE);

	if(dscale > cfx->MaxRadius){
		dscale = cfx->MaxRadius;
	}
	if(dscale < cfx->MinRadius){
		dscale = cfx->MinRadius;
	}

	// 1st
		VectorCopy( re->origin, p );
	if (re->rotation) {
		if ( cfx->cfxFlags & CFXF_PUFF_DONT_SCALE ) {
			VectorMA (p, re->radius, ru, p);	
			VectorMA (p, re->radius, rr, p);		
		}else if (  cfx->cfxFlags & CFXF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, sizedown, ru, p);	
			VectorMA (p, sizedown, rr, p);
		}else if (  cfx->cfxFlags & CFXF_DISTANCE_SCALE  ) {
			VectorMA (p, dscale, ru, p);	
			VectorMA (p, dscale, rr, p);
		}else{
			VectorMA (p, sizeup, ru, p);	
			VectorMA (p, sizeup, rr, p);
		}
	}else{
		if ( cfx->cfxFlags & CFXF_PUFF_DONT_SCALE ) {
			VectorMA (p, re->radius, rlvup, p);	
			VectorMA (p, re->radius, rlvright, p);		
		}else if (  cfx->cfxFlags & CFXF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, sizedown, rlvup, p);	
			VectorMA (p, sizedown, rlvright, p);
		}else if (  cfx->cfxFlags & CFXF_DISTANCE_SCALE  ) {
			VectorMA (p, dscale, rlvup, p);	
			VectorMA (p, dscale, rlvright, p);
		}else{
			VectorMA (p, sizeup, rlvup, p);	
			VectorMA (p, sizeup, rlvright, p);
		}
	}
		VectorCopy (p, verts[0].xyz);	
		verts[0].st[0] = 0;	
		verts[0].st[1] = 0;	
		verts[0].modulate[0] = re->shaderRGBA[0];
		verts[0].modulate[1] = re->shaderRGBA[1];
		verts[0].modulate[2] = re->shaderRGBA[2];
		verts[0].modulate[3] = re->shaderRGBA[3];


	// 2nd
		VectorCopy( re->origin, p );
	if (re->rotation) {
		if (  cfx->cfxFlags & CFXF_PUFF_DONT_SCALE  ) {
			VectorMA (p, re->radius, ru, p);	
			VectorMA (p, -re->radius, rr, p);
		}else if (  cfx->cfxFlags & CFXF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, sizedown, ru, p);	
			VectorMA (p, -sizedown, rr, p);
		}else if (  cfx->cfxFlags & CFXF_DISTANCE_SCALE  ) {
			VectorMA (p, dscale, ru, p);	
			VectorMA (p, -dscale, rr, p);
		}else{
			VectorMA (p, sizeup, ru, p);	
			VectorMA (p, -sizeup, rr, p);
		}
	}else{
		if (  cfx->cfxFlags & CFXF_PUFF_DONT_SCALE  ) {
			VectorMA (p, re->radius, rlvup, p);	
			VectorMA (p, -re->radius, rlvright, p);
		}else if (  cfx->cfxFlags & CFXF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, sizedown, rlvup, p);	
			VectorMA (p, -sizedown, rlvright, p);
		}else if (  cfx->cfxFlags & CFXF_DISTANCE_SCALE  ) {
			VectorMA (p, dscale, rlvup, p);	
			VectorMA (p, -dscale, rlvright, p);
		}else{
			VectorMA (p, sizeup, rlvup, p);	
			VectorMA (p, -sizeup, rlvright, p);
		}
	}
		VectorCopy (p, verts[1].xyz);	
		verts[1].st[0] = 0;	
		verts[1].st[1] = 1;	
		verts[1].modulate[0] = re->shaderRGBA[0];
		verts[1].modulate[1] = re->shaderRGBA[1];
		verts[1].modulate[2] = re->shaderRGBA[2];
		verts[1].modulate[3] = re->shaderRGBA[3];
		
	// 3rd
		VectorCopy( re->origin, p );
	if (re->rotation) {
		if (  cfx->cfxFlags & CFXF_PUFF_DONT_SCALE ) {
			VectorMA (p, -re->radius, ru, p);	
			VectorMA (p, -re->radius, rr, p);
		}else if (  cfx->cfxFlags & CFXF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, -sizedown, ru, p);	
			VectorMA (p, -sizedown, rr, p);
		}else if (  cfx->cfxFlags & CFXF_DISTANCE_SCALE  ) {
			VectorMA (p, -dscale, ru, p);	
			VectorMA (p, -dscale, rr, p);
		}else{
			VectorMA (p, -sizeup, ru, p);	
			VectorMA (p, -sizeup, rr, p);
		}
	}else{
		if (  cfx->cfxFlags & CFXF_PUFF_DONT_SCALE ) {
			VectorMA (p, -re->radius, rlvup, p);	
			VectorMA (p, -re->radius, rlvright, p);
		}else if (  cfx->cfxFlags & CFXF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, -sizedown, rlvup, p);	
			VectorMA (p, -sizedown, rlvright, p);
		}else if (  cfx->cfxFlags & CFXF_DISTANCE_SCALE  ) {
			VectorMA (p, -dscale, rlvup, p);	
			VectorMA (p, -dscale, rlvright, p);
		}else{
			VectorMA (p, -sizeup, rlvup, p);	
			VectorMA (p, -sizeup, rlvright, p);
		}
	}
		VectorCopy (p, verts[2].xyz);	
		verts[2].st[0] = 1;	
		verts[2].st[1] = 1;	
		verts[2].modulate[0] = re->shaderRGBA[0];
		verts[2].modulate[1] = re->shaderRGBA[1];
		verts[2].modulate[2] = re->shaderRGBA[2];
		verts[2].modulate[3] = re->shaderRGBA[3];
		
	// 4th
		VectorCopy( re->origin, p );
	if (re->rotation) {
		if (  cfx->cfxFlags & CFXF_PUFF_DONT_SCALE  ) {
			VectorMA (p, -re->radius, ru, p);	
			VectorMA (p, re->radius, rr, p);
		}else if (  cfx->cfxFlags & CFXF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, -sizedown, ru, p);	
			VectorMA (p, sizedown, rr, p);
		}else if (  cfx->cfxFlags & CFXF_DISTANCE_SCALE  ) {
			VectorMA (p, -dscale, ru, p);	
			VectorMA (p, dscale, rr, p);
		}else{
			VectorMA (p, -sizeup, ru, p);	
			VectorMA (p, sizeup, rr, p);
		}
	}else{
		if (  cfx->cfxFlags & CFXF_PUFF_DONT_SCALE  ) {
			VectorMA (p, -re->radius, rlvup, p);	
			VectorMA (p, re->radius, rlvright, p);
		}else if (  cfx->cfxFlags & CFXF_PUFF_SCALE_DOWN  ) {
			VectorMA (p, -sizedown, rlvup, p);	
			VectorMA (p, sizedown, rlvright, p);
		}else if (  cfx->cfxFlags & CFXF_DISTANCE_SCALE  ) {
			VectorMA (p, -dscale, rlvup, p);	
			VectorMA (p, dscale, rlvright, p);
		}else{
			VectorMA (p, -sizeup, rlvup, p);	
			VectorMA (p, sizeup, rlvright, p);
		}
	}
		VectorCopy (p, verts[3].xyz);	
		verts[3].st[0] = 1;	
		verts[3].st[1] = 0;	
		verts[3].modulate[0] = re->shaderRGBA[0];
		verts[3].modulate[1] = re->shaderRGBA[1];
		verts[3].modulate[2] = re->shaderRGBA[2];
		verts[3].modulate[3] = re->shaderRGBA[3];	
	// done

//	BG_EvaluateTrajectory( &cfx->pos, cg.time, re->origin );

	VectorSubtract( re->origin, cg.CameraFXView.vieworg, delta );

	len = VectorLength( delta );
	if ( len < cfx->radius ) {
		CG_FreeCameraFX( cfx );
		return;
	}

	if ( cg.time > cfx->endTime ) {
		CG_FreeCameraFX( cfx );
		return;
	}
	if(trace.fraction == 1.0){
		trap_R_AddPolyToScene( re->customShader, 4, verts );
	}
}

/*
===================
CG_AddCameraFX
===================
*/
void CG_AddCameraFX( void ) {
	cameraFX_t	*cfx, *next;

	// walk the list backwards, so any new local entities generated
	// (trails, marks, etc) will be present this frame
	cfx = cg_activeCameraFX.prev;

	for ( ; cfx != &cg_activeCameraFX ; cfx = next ) {
		// grab next now, so if the local entity is freed we
		// still have it
		next = cfx->prev;
		switch ( cfx->cfxType ) {
		default:
			CG_Error( "Bad cfxType: %i", cfx->cfxType );
			break;


		case CFX_CORONA:
			CG_Corona( cfx );
			break;
		}
	}
}
