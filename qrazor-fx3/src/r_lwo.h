/// ============================================================================
/*
Copyright (C) 2004 Robert Beckebans <trebor_7@users.sourceforge.net>
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
#ifndef R_LWO_H
#define R_LWO_H

/// includes ===================================================================
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
// system -------------------------------------------------------------------
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"

// xreal --------------------------------------------------------------------


/* chunk and subchunk IDs */

#define MAKE_ID(a,b,c,d) (((a)<<24)|((b)<<16)|((c)<<8)|(d))

#define ID_FORM  MAKE_ID('F','O','R','M')
#define ID_LWO2  MAKE_ID('L','W','O','2')
#define ID_LWOB  MAKE_ID('L','W','O','B')

/* top-level chunks */
#define ID_LAYR  MAKE_ID('L','A','Y','R')
#define ID_TAGS  MAKE_ID('T','A','G','S')
#define ID_PNTS  MAKE_ID('P','N','T','S')
#define ID_BBOX  MAKE_ID('B','B','O','X')
#define ID_VMAP  MAKE_ID('V','M','A','P')
#define ID_VMAD  MAKE_ID('V','M','A','D')
#define ID_POLS  MAKE_ID('P','O','L','S')
#define ID_PTAG  MAKE_ID('P','T','A','G')
#define ID_ENVL  MAKE_ID('E','N','V','L')
#define ID_CLIP  MAKE_ID('C','L','I','P')
#define ID_SURF  MAKE_ID('S','U','R','F')
#define ID_DESC  MAKE_ID('D','E','S','C')
#define ID_TEXT  MAKE_ID('T','E','X','T')
#define ID_ICON  MAKE_ID('I','C','O','N')

/* polygon types */
#define ID_FACE  MAKE_ID('F','A','C','E')
#define ID_CURV  MAKE_ID('C','U','R','V')
#define ID_PTCH  MAKE_ID('P','T','C','H')
#define ID_MBAL  MAKE_ID('M','B','A','L')
#define ID_BONE  MAKE_ID('B','O','N','E')

/* polygon tags */
#define ID_SURF  MAKE_ID('S','U','R','F')
#define ID_PART  MAKE_ID('P','A','R','T')
#define ID_SMGP  MAKE_ID('S','M','G','P')

/* envelopes */
#define ID_PRE   MAKE_ID('P','R','E',' ')
#define ID_POST  MAKE_ID('P','O','S','T')
#define ID_KEY   MAKE_ID('K','E','Y',' ')
#define ID_SPAN  MAKE_ID('S','P','A','N')
#define ID_TCB   MAKE_ID('T','C','B',' ')
#define ID_HERM  MAKE_ID('H','E','R','M')
#define ID_BEZI  MAKE_ID('B','E','Z','I')
#define ID_BEZ2  MAKE_ID('B','E','Z','2')
#define ID_LINE  MAKE_ID('L','I','N','E')
#define ID_STEP  MAKE_ID('S','T','E','P')

/* clips */
#define ID_STIL  MAKE_ID('S','T','I','L')
#define ID_ISEQ  MAKE_ID('I','S','E','Q')
#define ID_ANIM  MAKE_ID('A','N','I','M')
#define ID_XREF  MAKE_ID('X','R','E','F')
#define ID_STCC  MAKE_ID('S','T','C','C')
#define ID_TIME  MAKE_ID('T','I','M','E')
#define ID_CONT  MAKE_ID('C','O','N','T')
#define ID_BRIT  MAKE_ID('B','R','I','T')
#define ID_SATR  MAKE_ID('S','A','T','R')
#define ID_HUE   MAKE_ID('H','U','E',' ')
#define ID_GAMM  MAKE_ID('G','A','M','M')
#define ID_NEGA  MAKE_ID('N','E','G','A')
#define ID_IFLT  MAKE_ID('I','F','L','T')
#define ID_PFLT  MAKE_ID('P','F','L','T')

/* surfaces */
#define ID_COLR  MAKE_ID('C','O','L','R')
#define ID_LUMI  MAKE_ID('L','U','M','I')
#define ID_DIFF  MAKE_ID('D','I','F','F')
#define ID_SPEC  MAKE_ID('S','P','E','C')
#define ID_GLOS  MAKE_ID('G','L','O','S')
#define ID_REFL  MAKE_ID('R','E','F','L')
#define ID_RFOP  MAKE_ID('R','F','O','P')
#define ID_RIMG  MAKE_ID('R','I','M','G')
#define ID_RSAN  MAKE_ID('R','S','A','N')
#define ID_TRAN  MAKE_ID('T','R','A','N')
#define ID_TROP  MAKE_ID('T','R','O','P')
#define ID_TIMG  MAKE_ID('T','I','M','G')
#define ID_RIND  MAKE_ID('R','I','N','D')
#define ID_TRNL  MAKE_ID('T','R','N','L')
#define ID_BUMP  MAKE_ID('B','U','M','P')
#define ID_SMAN  MAKE_ID('S','M','A','N')
#define ID_SIDE  MAKE_ID('S','I','D','E')
#define ID_CLRH  MAKE_ID('C','L','R','H')
#define ID_CLRF  MAKE_ID('C','L','R','F')
#define ID_ADTR  MAKE_ID('A','D','T','R')
#define ID_SHRP  MAKE_ID('S','H','R','P')
#define ID_LINE  MAKE_ID('L','I','N','E')
#define ID_LSIZ  MAKE_ID('L','S','I','Z')
#define ID_ALPH  MAKE_ID('A','L','P','H')
#define ID_AVAL  MAKE_ID('A','V','A','L')
#define ID_GVAL  MAKE_ID('G','V','A','L')
#define ID_BLOK  MAKE_ID('B','L','O','K')

/* texture layer */
#define ID_TYPE  MAKE_ID('T','Y','P','E')
#define ID_CHAN  MAKE_ID('C','H','A','N')
#define ID_NAME  MAKE_ID('N','A','M','E')
#define ID_ENAB  MAKE_ID('E','N','A','B')
#define ID_OPAC  MAKE_ID('O','P','A','C')
#define ID_FLAG  MAKE_ID('F','L','A','G')
#define ID_PROJ  MAKE_ID('P','R','O','J')
#define ID_STCK  MAKE_ID('S','T','C','K')
#define ID_TAMP  MAKE_ID('T','A','M','P')

/* texture coordinates */
#define ID_TMAP  MAKE_ID('T','M','A','P')
#define ID_AXIS  MAKE_ID('A','X','I','S')
#define ID_CNTR  MAKE_ID('C','N','T','R')
#define ID_SIZE  MAKE_ID('S','I','Z','E')
#define ID_ROTA  MAKE_ID('R','O','T','A')
#define ID_OREF  MAKE_ID('O','R','E','F')
#define ID_FALL  MAKE_ID('F','A','L','L')
#define ID_CSYS  MAKE_ID('C','S','Y','S')

/* image map */
#define ID_IMAP  MAKE_ID('I','M','A','P')
#define ID_IMAG  MAKE_ID('I','M','A','G')
#define ID_WRAP  MAKE_ID('W','R','A','P')
#define ID_WRPW  MAKE_ID('W','R','P','W')
#define ID_WRPH  MAKE_ID('W','R','P','H')
#define ID_VMAP  MAKE_ID('V','M','A','P')
#define ID_AAST  MAKE_ID('A','A','S','T')
#define ID_PIXB  MAKE_ID('P','I','X','B')

/* procedural */
#define ID_PROC  MAKE_ID('P','R','O','C')
#define ID_COLR  MAKE_ID('C','O','L','R')
#define ID_VALU  MAKE_ID('V','A','L','U')
#define ID_FUNC  MAKE_ID('F','U','N','C')
#define ID_FTPS  MAKE_ID('F','T','P','S')
#define ID_ITPS  MAKE_ID('I','T','P','S')
#define ID_ETPS  MAKE_ID('E','T','P','S')

/* gradient */
#define ID_GRAD  MAKE_ID('G','R','A','D')
#define ID_GRST  MAKE_ID('G','R','S','T')
#define ID_GREN  MAKE_ID('G','R','E','N')
#define ID_PNAM  MAKE_ID('P','N','A','M')
#define ID_INAM  MAKE_ID('I','N','A','M')
#define ID_GRPT  MAKE_ID('G','R','P','T')
#define ID_FKEY  MAKE_ID('F','K','E','Y')
#define ID_IKEY  MAKE_ID('I','K','E','Y')

/* shader */
#define ID_SHDR  MAKE_ID('S','H','D','R')
#define ID_DATA  MAKE_ID('D','A','T','A')


typedef char            I1;
typedef short           I2;
typedef int             I4;
typedef unsigned char   U1;
typedef unsigned short  U2;
typedef unsigned int    U4;
typedef float           F4;
typedef unsigned int    VX;
typedef float           FP4;
typedef float           ANG4;
typedef float           VEC12[3];
typedef float           COL12[3];
typedef char            ID4[5];
typedef char            S0[255];
typedef char            FNAM0[255];


class r_lwo_model_c : public r_static_model_c
{
public:
	//
	// constructor / destructor
	//
	r_lwo_model_c(const std::string &name, byte *buffer, uint_t buffer_size);
	virtual ~r_lwo_model_c();
	
	//
	// virtual functions
	//
	virtual void	load();
	
private:
	ushort_t	readU2();
	uint_t		readU4();
	
	float		readF4();
	
	void		readID4(char *id);
	void		readVEC12(vec3_c &vec);
	void		readVX(index_t &vx);
	void		readName(char *name);
	
	void		readTags(uint_t nbytes);
	r_mesh_c*	readLayr(uint_t nbytes);
	void		readPnts(uint_t nbytes, r_mesh_c *mesh);
	void		readBbox(uint_t nbytes, r_mesh_c *mesh);
	void		readPols(uint_t nbytes, r_mesh_c *mesh);
	
	uint_t	_readcount;
	uint_t	_readcount_old;
};


#endif // R_LWO_H
