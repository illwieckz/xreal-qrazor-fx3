/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
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


/// includes ===================================================================
// system -------------------------------------------------------------------
#include <boost/spirit/core.hpp>
#include <boost/spirit/utility.hpp>
#include <boost/spirit/utility/lists.hpp>
#include <boost/spirit/utility/escape_char.hpp>

// qrazor-fx ----------------------------------------------------------------
#include "map_local.h"

#include "common.h"
#include "cmd.h"
#include "vfs.h"




//brush_texture_t		side_brushtextures[MAX_MAP_SIDES];

aabb_c			map_bbox;


int		c_boxbevels;
int		c_edgebevels;

int		c_areaportals;

int		c_clipbrushes;




static int	CreateNewFloatPlane(const cplane_c &p)
{
	cplane_c*	planes[2];

	if(p._normal.length() < 0.5)
		Com_Error(ERR_FATAL, "CreateNewFloatPlane: bad normal");
		
	// create a new plane
	//if(nummapplanes+2 > MAX_MAP_PLANES)
	//	Com_Error(ERR_FATAL, "MAX_MAP_PLANES");

	//p = &mapplanes[nummapplanes];
	//Vector3_Copy(normal, p->normal);
	//p->dist = dist;
	//p->type = (p+1)->type = PlaneTypeForNormal(p._normal);
	
	planes[0] = new cplane_c(p);
	planes[1] = new cplane_c(p);
	
	//VectorSubtract (vec3_origin, normal, (p+1)._normal);
	//(p+1)->dist = -dist;
	
	planes[1]->negate();

	//nummapplanes += 2;

	// allways put axial planes facing positive first
	if(p.getType() < 3)
	{
		if(p._normal[0] < 0 || p._normal[1] < 0 || p._normal[2] < 0)
		{
			// flip order
			map_planes.push_back(planes[1]);
			map_planes.push_back(planes[0]);
			
			return map_planes.size() - 1;
		}
	}
	
	map_planes.push_back(planes[0]);
	map_planes.push_back(planes[1]);

	return map_planes.size() - 2;
}


static int	FindFloatPlane(cplane_c &p)
{
	p.snap();
	
	for(unsigned int i=0; i<map_planes.size(); i++)
	{
		if(p == *map_planes[i])
			return i;
	}

	return CreateNewFloatPlane(p);
}

int	PlaneFromPoints(const vec3_c &p0, const vec3_c &p1, const vec3_c &p2)
{
	cplane_c p;
	
	p.fromThreePointForm(p0, p1, p2);

	return FindFloatPlane(p);
}


/*
int	SetBrushContents(mapbrush_t *b)
{
	int			contents;
	side_t*			s;
	int			i;
	int			trans;

	s = &b->sides[0];
	contents = s->contents;
	trans = texinfo[s->texinfo].flags;
	for (i=1 ; i<b->numsides ; i++, s++)
	{
		s = &b->original_sides[i];
		trans |= texinfo[s->texinfo].flags;
		if (s->contents != contents)
		{
			printf ("Entity %i, Brush %i: mixed face contents\n"
				, b->entitynum, b->brushnum);
			break;
		}
	}

	// if any side is translucent, mark the contents
	// and change solid to window
	if ( trans & (SURF_TRANS33|SURF_TRANS66) )
	{
		contents |= CONTENTS_TRANSLUCENT;
		if (contents & CONTENTS_SOLID)
		{
			contents &= ~CONTENTS_SOLID;
			contents |= CONTENTS_WINDOW;
		}
	}

	return contents;
}
*/



/*
=================
AddBrushBevels

Adds any additional planes necessary to allow the brush to be expanded
against axial bounding boxes
=================
*/
/*
void AddBrushBevels (mapbrush_t *b)
{
	int		axis, dir;
	int		i, j, k, l, order;
	side_t	sidetemp;
	brush_texture_t	tdtemp;
	side_t	*s, *s2;
	vec3_t	normal;
	float	dist;
	winding_t	*w, *w2;
	vec3_t	vec, vec2;
	float	d;

	//
	// add the axial planes
	//
	order = 0;
	for (axis=0 ; axis <3 ; axis++)
	{
		for (dir=-1 ; dir <= 1 ; dir+=2, order++)
		{
			// see if the plane is allready present
			for (i=0, s=b->original_sides ; i<b->numsides ; i++,s++)
			{
				if (mapplanes[s->planenum].normal[axis] == dir)
					break;
			}

			if (i == b->numsides)
			{	// add a new side
				if (nummapbrushsides == MAX_MAP_BRUSHSIDES)
					Error ("MAX_MAP_BRUSHSIDES");
				nummapbrushsides++;
				b->numsides++;
				VectorClear (normal);
				normal[axis] = dir;
				if (dir == 1)
					dist = b->maxs[axis];
				else
					dist = -b->mins[axis];
				s->planenum = FindFloatPlane (normal, dist);
				s->texinfo = b->original_sides[0].texinfo;
				s->contents = b->original_sides[0].contents;
				s->bevel = true;
				c_boxbevels++;
			}

			// if the plane is not in it canonical order, swap it
			if (i != order)
			{
				sidetemp = b->original_sides[order];
				b->original_sides[order] = b->original_sides[i];
				b->original_sides[i] = sidetemp;

				j = b->original_sides - brushsides;
				tdtemp = side_brushtextures[j+order];
				side_brushtextures[j+order] = side_brushtextures[j+i];
				side_brushtextures[j+i] = tdtemp;
			}
		}
	}

	//
	// add the edge bevels
	//
	if (b->numsides == 6)
		return;		// pure axial

	// test the non-axial plane edges
	for (i=6 ; i<b->numsides ; i++)
	{
		s = b->original_sides + i;
		w = s->winding;
		if (!w)
			continue;
		for (j=0 ; j<w->numpoints ; j++)
		{
			k = (j+1)%w->numpoints;
			VectorSubtract (w->p[j], w->p[k], vec);
			if (VectorNormalize (vec, vec) < 0.5)
				continue;
			SnapVector (vec);
			for (k=0 ; k<3 ; k++)
				if ( vec[k] == -1 || vec[k] == 1)
					break;	// axial
			if (k != 3)
				continue;	// only test non-axial edges

			// try the six possible slanted axials from this edge
			for (axis=0 ; axis <3 ; axis++)
			{
				for (dir=-1 ; dir <= 1 ; dir+=2)
				{
					// construct a plane
					VectorClear (vec2);
					vec2[axis] = dir;
					CrossProduct (vec, vec2, normal);
					if (VectorNormalize (normal, normal) < 0.5)
						continue;
					dist = DotProduct (w->p[j], normal);

					// if all the points on all the sides are
					// behind this plane, it is a proper edge bevel
					for (k=0 ; k<b->numsides ; k++)
					{
						// if this plane has allready been used, skip it
						if (PlaneEqual (&mapplanes[b->original_sides[k].planenum]
							, normal, dist) )
							break;

						w2 = b->original_sides[k].winding;
						if (!w2)
							continue;
						for (l=0 ; l<w2->numpoints ; l++)
						{
							d = DotProduct (w2->p[l], normal) - dist;
							if (d > 0.1)
								break;	// point in front
						}
						if (l != w2->numpoints)
							break;
					}

					if (k != b->numsides)
						continue;	// wasn't part of the outer hull
					// add this plane
					if (nummapbrushsides == MAX_MAP_BRUSHSIDES)
						Error ("MAX_MAP_BRUSHSIDES");
					nummapbrushsides++;
					s2 = &b->original_sides[b->numsides];
					s2->planenum = FindFloatPlane (normal, dist);
					s2->texinfo = b->original_sides[0].texinfo;
					s2->contents = b->original_sides[0].contents;
					s2->bevel = true;
					c_edgebevels++;
					b->numsides++;
				}
			}
		}
	}
}
*/


/*
================
MakeBrushWindings

makes basewindigs for sides and mins / maxs for the brush
================
*/
/*
bool MakeBrushWindings (mapbrush_t *ob)
{
	int			i, j;
	winding_t	*w;
	side_t		*side;
	plane_t		*plane;

	ClearBounds (ob->mins, ob->maxs);

	for (i=0 ; i<ob->numsides ; i++)
	{
		plane = &mapplanes[ob->original_sides[i].planenum];
		w = BaseWindingForPlane (plane->normal, plane->dist);
		for (j=0 ; j<ob->numsides && w; j++)
		{
			if (i == j)
				continue;
			if (ob->original_sides[j].bevel)
				continue;
			plane = &mapplanes[ob->original_sides[j].planenum^1];
			ChopWindingInPlace (&w, plane->normal, plane->dist, 0); //CLIP_EPSILON);
		}

		side = &ob->original_sides[i];
		side->winding = w;
		if (w)
		{
			side->visible = true;
			for (j=0 ; j<w->numpoints ; j++)
				AddPointToBounds (w->p[j], ob->mins, ob->maxs);
		}
	}

	for (i=0 ; i<3 ; i++)
	{
		if (ob->mins[0] < -4096 || ob->maxs[0] > 4096)
			printf ("entity %i, brush %i: bounds out of range\n", ob->entitynum, ob->brushnum);
		if (ob->mins[0] > 4096 || ob->maxs[0] < -4096)
			printf ("entity %i, brush %i: no visible sides on brush\n", ob->entitynum, ob->brushnum);
	}

	return true;
}
*/

/*
=================
ParseBrush
=================
*/
/*
void ParseBrush (entity_t *mapent)
{
	mapbrush_t		*b;
	int			i,j, k;
	int			mt;
	side_t		*side, *s2;
	int			planenum;
	//brush_texture_t	td;
	int			planepts[3][3];

	if (nummapbrushes == MAX_MAP_BRUSHES)
		Error ("nummapbrushes == MAX_MAP_BRUSHES");

	b = &mapbrushes[nummapbrushes];
	b->original_sides = &brushsides[nummapbrushsides];
	b->entitynum = num_entities-1;
	b->brushnum = nummapbrushes - mapent->firstbrush;

	do
	{
		if (!GetToken (true))
			break;
		if (!strcmp (token, "}") )
			break;

		if (nummapbrushsides == MAX_MAP_BRUSHSIDES)
			Error ("MAX_MAP_BRUSHSIDES");
		side = &brushsides[nummapbrushsides];

		// read the three point plane definition
		for (i=0 ; i<3 ; i++)
		{
			if (i != 0)
				GetToken (true);
			if (strcmp (token, "(") )
				Error ("parsing brush");
			
			for (j=0 ; j<3 ; j++)
			{
				GetToken (false);
				planepts[i][j] = atoi(token);
			}
			
			GetToken (false);
			if (strcmp (token, ")") )
				Error ("parsing brush");
				
		}


		//
		// read the texturedef
		//
		GetToken (false);
		strcpy (td.name, token);

		GetToken (false);
		td.shift[0] = atoi(token);
		GetToken (false);
		td.shift[1] = atoi(token);
		GetToken (false);
		td.rotate = atoi(token);	
		GetToken (false);
		td.scale[0] = atof(token);
		GetToken (false);
		td.scale[1] = atof(token);

		// find default flags and values
		mt = FindMiptex (td.name);
		td.flags = textureref[mt].flags;
		td.value = textureref[mt].value;
		side->contents = textureref[mt].contents;
		side->surf = td.flags = textureref[mt].flags;

		if (TokenAvailable())
		{
			GetToken (false);
			side->contents = atoi(token);
			GetToken (false);
			side->surf = td.flags = atoi(token);
			GetToken (false);
			td.value = atoi(token);
		}

		// translucent objects are automatically classified as detail
		if (side->surf & (SURF_TRANS33|SURF_TRANS66) )
			side->contents |= CONTENTS_DETAIL;
		if (side->contents & (CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP) )
			side->contents |= CONTENTS_DETAIL;
		if (fulldetail)
			side->contents &= ~CONTENTS_DETAIL;
		if (!(side->contents & ((LAST_VISIBLE_CONTENTS-1) 
			| CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP|CONTENTS_MIST)  ) )
			side->contents |= CONTENTS_SOLID;

		// hints and skips are never detail, and have no content
		if (side->surf & (SURF_HINT|SURF_SKIP) )
		{
			side->contents = 0;
			side->surf &= ~CONTENTS_DETAIL;
		}


		//
		// find the plane number
		//
		planenum = PlaneFromPoints (planepts[0], planepts[1], planepts[2]);
		if (planenum == -1)
		{
			printf ("Entity %i, Brush %i: plane with no normal\n"
				, b->entitynum, b->brushnum);
			continue;
		}

		//
		// see if the plane has been used already
		//
		for (k=0 ; k<b->numsides ; k++)
		{
			s2 = b->original_sides + k;
			if (s2->planenum == planenum)
			{
				printf ("Entity %i, Brush %i: duplicate plane\n"
					, b->entitynum, b->brushnum);
				break;
			}
			if ( s2->planenum == (planenum^1) )
			{
				printf ("Entity %i, Brush %i: mirrored plane\n"
					, b->entitynum, b->brushnum);
				break;
			}
		}
		if (k != b->numsides)
			continue;		// duplicated

		//
		// keep this side
		//

		side = b->original_sides + b->numsides;
		side->planenum = planenum;
		side->texinfo = TexinfoForBrushTexture (&mapplanes[planenum],
			&td, vec3_origin);

		// save the td off in case there is an origin brush and we
		// have to recalculate the texinfo
		side_brushtextures[nummapbrushsides] = td;

		nummapbrushsides++;
		b->numsides++;
	} while (1);

	// get the content for the entire brush
	b->contents = BrushContents (b);

	// allow detail brushes to be removed 
	if (nodetail && (b->contents & CONTENTS_DETAIL) )
	{
		b->numsides = 0;
		return;
	}

	// allow water brushes to be removed
	if (nowater && (b->contents & (CONTENTS_LAVA | CONTENTS_SLIME | CONTENTS_WATER)) )
	{
		b->numsides = 0;
		return;
	}

	// create windings for sides and bounds for brush
	MakeBrushWindings (b);

	// brushes that will not be visible at all will never be
	// used as bsp splitters
	if (b->contents & (CONTENTS_PLAYERCLIP|CONTENTS_MONSTERCLIP) )
	{
		c_clipbrushes++;
		for (i=0 ; i<b->numsides ; i++)
			b->original_sides[i].texinfo = TEXINFO_NODE;
	}

	//
	// origin brushes are removed, but they set
	// the rotation origin for the rest of the brushes
	// in the entity.  After the entire entity is parsed,
	// the planenums and texinfos will be adjusted for
	// the origin brush
	//
	if (b->contents & CONTENTS_ORIGIN)
	{
		char	string[32];
		vec3_t	origin;

		if (num_entities == 1)
		{
			Error ("Entity %i, Brush %i: origin brushes not allowed in world"
				, b->entitynum, b->brushnum);
			return;
		}

		VectorAdd (b->mins, b->maxs, origin);
		VectorScale (origin, 0.5, origin);

		sprintf (string, "%i %i %i", (int)origin[0], (int)origin[1], (int)origin[2]);
		SetKeyValue (&entities[b->entitynum], "origin", string);

		VectorCopy (origin, entities[b->entitynum].origin);

		// don't keep this brush
		b->numsides = 0;

		return;
	}

	AddBrushBevels (b);

	nummapbrushes++;
	mapent->numbrushes++;		
}
*/

/*
================
MoveBrushesToWorld

Takes all of the brushes from the current entity and
adds them to the world's brush list.

Used by func_group and func_areaportal
================
*/
/*
void	MoveBrushesToWorld(entity_t *mapent)
{
	int			newbrushes;
	int			worldbrushes;
	mapbrush_t*		temp;
	int			i;

	// this is pretty gross, because the brushes are expected to be
	// in linear order for each entity

	newbrushes = mapent->numbrushes;
	worldbrushes = entities[0].numbrushes;

	temp = malloc(newbrushes*sizeof(mapbrush_t));
	memcpy (temp, mapbrushes + mapent->firstbrush, newbrushes*sizeof(mapbrush_t));

#if	0		// let them keep their original brush numbers
	for (i=0 ; i<newbrushes ; i++)
		temp[i].entitynum = 0;
#endif

	// make space to move the brushes (overlapped copy)
	memmove (mapbrushes + worldbrushes + newbrushes,
		mapbrushes + worldbrushes,
		sizeof(mapbrush_t) * (nummapbrushes - worldbrushes - newbrushes) );

	// copy the new brushes down
	memcpy (mapbrushes + worldbrushes, temp, sizeof(mapbrush_t) * newbrushes);

	// fix up indexes
	entities[0].numbrushes += newbrushes;
	for (i=1 ; i<num_entities ; i++)
		entities[i].firstbrush += newbrushes;
	free (temp);

	mapent->numbrushes = 0;
}
*/

/*
static void	AdjustBrushesForOrigin(entity_t &ent)
{
	for(std::vector<mapbrush_t*>::const_iterator ir = ent.brushes.begin(); ir != ent.brushes.end(); ++ir)
	{
		mapbrush_t* b = *ir;
		
		for(int j=0; j<b->sides_num; j++)
		{
			side_t &s = b->sides[j];
			
			vec_t newdist = map_planes[s.planenum]._dist - map_planes[s.planenum]._normal.dotProduct(ent.origin);
			
			map_planes[s.planenum]._dist = newdist;
			
			s.planenum = FindFloatPlane(map_planes[s.planenum]);
			//s->texinfo = TexinfoForBrushTexture (&mapplanes[s->planenum], &side_brushtextures[s-brushsides], mapent->origin);
		}
			
		//TODO
		//MakeBrushWindings(b);
	}
}


static void	ParseBrushDefPrimitive(entity_t &ent, char **data_p)
{
	//TODO
	
	while(true)
	{
		char *token = Com_Parse(data_p);
		
		if(!token[0] && !*data_p)
			Com_Error(ERR_FATAL, "ParseMapEntity: EOF without closing brace");
	
		if(token[0] == '}')
				break;
	}
		
	Com_Parse(data_p);
}

static bool	ParseMapEntity(char **data_p)
{
	entity_t	mapent;
	
	char*		token;
	
	std::string	key;
	std::string	value;
	
	token = Com_Parse(data_p);

	if(!token[0])
		return false;

	if(!X_strequal(token, "{"))
		Com_Error(ERR_FATAL, "ParseMapEntity: found '%s' instead of {", token);
	
//	mapent->portalareas[0] = -1;
//	mapent->portalareas[1] = -1;

	do
	{
		token = Com_Parse(data_p);
		
		if(!token[0])
		{
			Com_Error(ERR_FATAL, "ParseMapEntity: EOF without closing brace");
			return false;
		}
		
		if(X_strequal(token, "}"))
			break;
		
		if(X_strequal(token, "{"))
		{
			token = Com_Parse(data_p);
			
			if(X_strequal(token, "brushDef"))
			{
				ParseBrushDefPrimitive(mapent, data_p);
			}
			else
			{
				Com_Error(ERR_FATAL, "ParseMapEntity: unknown primitive '%s'", token);
			}
		}
		else
		{
			key = token;
		
			token = Com_Parse(data_p);
		
			std::string value = token;
		
			if(!*data_p)
				Com_Error(ERR_FATAL, "ParseMapEntity: EOF without closing brace");
	
			if(token[0] == '}')
				Com_Error(ERR_FATAL, "ParseMapEntity: closing brace without data");
		
			mapent.epairs.insert(make_pair(key, value));
		}
		
	}while(true);

	GetVector3ForKey(mapent, "origin", mapent.origin);


	//
	// if there was an origin brush, offset all of the planes and texinfo
	//
	if(mapent.origin[0] || mapent.origin[1] || mapent.origin[2])
	{
		AdjustBrushesForOrigin(mapent);
	}

	

	// group entities are just for editor convenience
	// toss all brushes into the world entity
	if(!strcmp("func_group", ValueForKey(mapent, "classname")))
	{
		//MoveBrushesToWorld(mapent);
		mapent.brushes.clear();
		return true;
	}

	// areaportal entities move their brushes, but don't eliminate
	// the entity
	
	if (!strcmp ("func_areaportal", ValueForKey (mapent, "classname")))
	{
		char	str[128];

		if (mapent->numbrushes != 1)
			Error ("Entity %i: func_areaportal can only be a single brush", num_entities-1);

		b = &mapbrushes[nummapbrushes-1];
		b->contents = CONTENTS_AREAPORTAL;
		c_areaportals++;
		mapent->areaportalnum = c_areaportals;
		// set the portal number as "style"
		sprintf (str, "%i", c_areaportals);
		SetKeyValue (mapent, "style", str);
		MoveBrushesToWorld (mapent);
		return true;
	}
	
	
	entities.push_back(mapent);

	return true;
}
*/

static map_entity_c*	map_entity;

static map_brush_c*		map_brush;
static map_brush_side_c*		map_brushside;
static cplane_c*				map_plane;
static std::string				map_shader;

static std::string		map_key;
static std::string		map_value;

static float		map_float0;
static float		map_float1;
static float		map_float2;
static float		map_float3;


static void	MAP_Version(int version)
{
	if(version != MAP_VERSION)
		Com_Error(ERR_DROP, "MAP_Version: wrong version number (%i should be %i)", version, MAP_VERSION);
}

static void	MAP_NewEntity(char begin)
{
	Com_Printf("parsing entity %i ...\n", entities.size());
	
	entities.push_back(map_entity_c());
	
	map_entity = &entities[entities.size()-1];
}

static void	MAP_FinishEntity(char const* begin, char const* end)
{
	map_entity->toString();
}

static void	MAP_BrushDef3(char const* begin, char const* end)
{
	Com_Printf("MAP_BrushDef3()\n");
	
	map_brush = new map_brush_c(entities.size()-1);
	map_brushes.push_back(map_brush);
	
	map_entity->addBrush(map_brush);
}

static void	MAP_BrushDef3Side(char const* begin, char const* end)
{
	Com_Printf("MAP_BrushDef3Side()\n");
	
	map_brushside = new map_brush_side_c();
	map_brushsides.push_back(map_brushside);
	
	map_brushside->setPlane(map_plane);
	map_brushside->setShader(map_shader);
	
	map_brush->addSide(map_brushside);
}

static void	MAP_PlaneEQ(char const* begin, char const* end)
{
	Com_Printf("MAP_PlaneEQ()\n");
	
	map_planes.push_back(new cplane_c(map_float0, map_float1, map_float2, -map_float3));
	
	
}

static void	MAP_KeyValueInfo(char const* begin, char const* end)
{
	//Com_Printf("'%s' '%s'\n", map_key.c_str(), map_value.c_str());
	map_entity->setKeyValue(map_key, map_value);
}


struct map_grammar_t : public boost::spirit::grammar<map_grammar_t>
{
	template <typename ScannerT>
	struct definition
	{
        	definition(map_grammar_t const& self)
		{
			// start grammar definition
			restofline
				=	boost::spirit::lexeme_d[boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - (boost::spirit::eol_p | boost::spirit::ch_p('}'))]]
				;
				
			skip_restofline
				=	restofline
				;
				
			skip_block
				=	boost::spirit::ch_p('{') >>
					*skip_restofline >>
					boost::spirit::ch_p('}')
				;
			
			entity
				=
					boost::spirit::ch_p('{')[&MAP_NewEntity] >>
					*(primitive | key_value[&MAP_KeyValueInfo]) >>
					boost::spirit::ch_p('}')
				;
				
			primitive
				=	boost::spirit::ch_p('{') >>
					brushDef3 >>
					boost::spirit::ch_p('}')
				;
				
			brushDef3
				=	boost::spirit::str_p("brushDef3")[&MAP_BrushDef3] >>
					boost::spirit::ch_p('{') >>
					+brushDef3_side[&MAP_BrushDef3Side] >>
					boost::spirit::ch_p('}')
				;
				
			brushDef3_side
				=	plane_eq[&MAP_PlaneEQ] >>
					tex_mat >>
					shader >>
					detail_flags
				;
				
			plane_eq
				=	boost::spirit::ch_p('(') >>
					boost::spirit::real_p[boost::spirit::assign(map_float0)] >>
					boost::spirit::real_p[boost::spirit::assign(map_float1)] >>
					boost::spirit::real_p[boost::spirit::assign(map_float2)] >>
					boost::spirit::real_p[boost::spirit::assign(map_float3)] >>
					boost::spirit::ch_p(')')
				;
				
			tex_mat
				=	boost::spirit::ch_p('(') >>
					boost::spirit::ch_p('(') >>
					boost::spirit::real_p[boost::spirit::assign(map_float0)] >>
					boost::spirit::real_p[boost::spirit::assign(map_float1)] >>
					boost::spirit::real_p[boost::spirit::assign(map_float2)] >>
					boost::spirit::ch_p(')') >>
					boost::spirit::ch_p('(') >>
					boost::spirit::real_p[boost::spirit::assign(map_float0)] >>
					boost::spirit::real_p[boost::spirit::assign(map_float1)] >>
					boost::spirit::real_p[boost::spirit::assign(map_float2)] >>
					boost::spirit::ch_p(')') >>
					boost::spirit::ch_p(')')
				;
				
			detail_flags
				=	boost::spirit::int_p >>
					boost::spirit::int_p >>
					boost::spirit::int_p
				;
				
			shader
				=	boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][boost::spirit::assign(map_shader)] >>
					boost::spirit::ch_p('\"')
				;
				
			key_value
				=	boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][boost::spirit::assign(map_key)] >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::ch_p('\"') >>
					boost::spirit::refactor_unary_d[+boost::spirit::anychar_p - boost::spirit::ch_p('\"')][boost::spirit::assign(map_value)] >>
					boost::spirit::ch_p('\"')
				;
				
			expression
				=	boost::spirit::str_p("Version") >> boost::spirit::int_p[&MAP_Version] >>
					+entity[&MAP_FinishEntity] >>
					*boost::spirit::anychar_p
				;
				
			// end grammar definiton
		}
		
		boost::spirit::rule<ScannerT>	restofline,
						skip_restofline,
						skip_block,
						
						entity,
							primitive,
								brushDef3,
									brushDef3_side,
										plane_eq,
										tex_mat,
										shader,
										detail_flags,
							key_value,
						expression;
		
		boost::spirit::rule<ScannerT> const&
		start() const { return expression; }
	};
};


void	LoadMapFile(const std::string &filename)
{
	char*		buf = NULL;
				
	Com_Printf("------- LoadMapFile -------\n");
	
	VFS_FLoad(filename, (void **)&buf);
	if(!buf)
	{
		Com_Error(ERR_FATAL, "LoadMapFile: couldn't load %s", filename.c_str());
		return;
	}
	Com_Printf("loading '%s' ...\n", filename.c_str());

	map_brushsides.clear();
	entities.clear();
	
	map_grammar_t	grammar;
	
	boost::spirit::parse_info<> info = boost::spirit::parse
	(
		buf,
		grammar,
		boost::spirit::space_p ||
		boost::spirit::comment_p("/*", "*/") ||
		boost::spirit::comment_p("//")
	);
	
	if(!info.full)
	{
		Com_Error(ERR_FATAL, "LoadMapFile: parsing failed");
	}

	if(entities.empty())
	{
		Com_Error(ERR_FATAL, "LoadMapFile: no entities");
		return;
	}
	
	// compute world bounding box
	map_bbox.clear();
	
	for(unsigned int i=0; i<entities[0].getBrushes().size(); i++)
	{
		map_bbox.mergeWith(map_brushes[i]->getAABB());
	}

	Com_Printf("%5i brushes\n",	map_brushes.size());
	Com_Printf("%5i clipbrushes\n",	c_clipbrushes);
	Com_Printf("%5i total sides\n",	map_brushsides.size());
	Com_Printf("%5i boxbevels\n",	c_boxbevels);
	Com_Printf("%5i edgebevels\n",	c_edgebevels);
	Com_Printf("%5i entities\n",	entities.size());
	Com_Printf("%5i planes\n",	map_planes.size());
	Com_Printf("%5i areaportals\n",	c_areaportals);
	Com_Printf("size: %s\n",	map_bbox.toString());

	VFS_FFree(buf);
}


