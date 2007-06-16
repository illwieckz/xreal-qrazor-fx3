#include "cg_local.h"

/*
===================
CG_InitMapFX

This is called at startup and for tournement restarts
===================
*/
void CG_InitMapFX(void)
{
	int             i;

	memset(cg_mapFX, 0, sizeof(cg_mapFX));
	cg_activeMapFX.next = &cg_activeMapFX;
	cg_activeMapFX.prev = &cg_activeMapFX;
	cg_freeMapFX = cg_mapFX;
	for(i = 0; i < MAX_MAPFX - 1; i++)
	{
		cg_mapFX[i].next = &cg_mapFX[i + 1];
	}

}


/*
==================
cg_freeMapFX
==================
*/
void CG_FreeMapFX(mapFX_t * mfx)
{
	if(!mfx->prev)
	{
		CG_Error("cg_freeMapFX: not active");
	}

	// remove from the doubly linked active list
	mfx->prev->next = mfx->next;
	mfx->next->prev = mfx->prev;

	// the free list is only singly linked
	mfx->next = cg_freeMapFX;
	cg_freeMapFX = mfx;
}

/*
===================
CG_AllocCameraFX

Will allways succeed, even if it requires freeing an old active entity
===================
*/
mapFX_t        *CG_AllocMapFX(void)
{
	mapFX_t        *mfx;

	if(!cg_freeMapFX)
	{
		// no free entities, so free the one at the end of the chain
		// remove the oldest active entity
		if(!(cg_activeMapFX.prev->mfxFlags & MFXF_NEVER_FREE))
		{
			CG_FreeMapFX(cg_activeMapFX.prev);
		}
	}

	mfx = cg_freeMapFX;
	cg_freeMapFX = cg_freeMapFX->next;

	memset(mfx, 0, sizeof(*mfx));

	// link into the active list
	mfx->next = cg_activeMapFX.next;
	mfx->prev = &cg_activeMapFX;
	cg_activeMapFX.next->prev = mfx;
	cg_activeMapFX.next = mfx;
	return mfx;
}

//==============================================================================



/*
===================
CG_AddMapFX
===================
*/
void CG_AddMapFX(void)
{
	mapFX_t        *mfx, *next;

	// walk the list backwards, so any new local entities generated
	// (trails, marks, etc) will be present this frame
	mfx = cg_activeMapFX.prev;

	for(; mfx != &cg_activeMapFX; mfx = next)
	{
		// grab next now, so if the local entity is freed we
		// still have it
		next = mfx->prev;
		switch (mfx->mfxType)
		{
			default:
				CG_Error("Bad mfxType: %i", mfx->mfxType);
				break;


			case MFX_CORONA:
//          CG_Corona( mfx );
				break;
		}
	}
}
