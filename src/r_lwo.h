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
	void		readCOL12(vec3_c &vec);
	index_t		readVX();
	void		readName(char *name);
	
	void		readTags(uint_t nbytes);
	void		readLayr(uint_t nbytes, matrix_c &transform);
	void		readPnts(uint_t nbytes, std::vector<vec3_c> &vertexes);
	void		readBbox(uint_t nbytes, aabb_c &bbox);
	void		readPols(uint_t nbytes, std::vector<index_t> &indexes);
	void		readPtag(uint_t nbytes, std::vector<lwo_ptag_t> &ptags);
	void		readVmap(uint_t nbytes, std::vector<lwo_vmap_t> &vmaps);
	void		readVmad(uint_t nbytes, std::vector<lwo_vmad_t> &vmads);
	
	
	uint_t			_readcount;
	uint_t			_readcount_old;
};


#endif // R_LWO_H
