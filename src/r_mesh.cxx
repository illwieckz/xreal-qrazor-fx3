/// ============================================================================
/*
Copyright (C) 1997-2001 Id Software, Inc.
Copyright (C) 2003 Robert Beckebans <trebor_7@users.sourceforge.net>
Copyright (C) 2003, 2004  contributors of the XreaL project
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
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"


r_mesh_c::r_mesh_c()
{
	bbox.clear();

	vbo_array_buffer	= 0;
	vbo_vertexes_ofs	= 0;
	vbo_texcoords_ofs	= 0;
	vbo_texcoords_lm_ofs	= 0;
	vbo_tangents_ofs	= 0;
	vbo_binormals_ofs	= 0;
	vbo_normals_ofs		= 0;
	vbo_lights_ofs		= 0;
	vbo_colors_ofs		= 0;

	vbo_element_array_buffer	= 0;
	vbo_indexes_ofs			= 0;
}

r_mesh_c::~r_mesh_c()
{
	if(vbo_array_buffer)
	{
		xglDeleteBuffersARB(1, &vbo_array_buffer);
	}

	if(vbo_element_array_buffer)
	{
		xglDeleteBuffersARB(1, &vbo_element_array_buffer);
	}
}


void	r_mesh_c::fillVertexes(int vertexes_num, bool with_lightmap_texcoords, bool with_light_vectors)
{
	vertexes = std::vector<vec3_c>(vertexes_num);
	texcoords = std::vector<vec2_c>(vertexes_num);
	
	if(with_lightmap_texcoords)
		texcoords_lm = std::vector<vec2_c>(vertexes_num);
	
	tangents = std::vector<vec3_c>(vertexes_num);
	binormals = std::vector<vec3_c>(vertexes_num);
	normals = std::vector<vec3_c>(vertexes_num);
	
	if(with_light_vectors)
		lights= std::vector<vec3_c>(vertexes_num);
	
	colors = std::vector<vec4_c>(vertexes_num, color_white);
}

bool	r_mesh_c::isNotValid() const
{
	if(vertexes.empty() || indexes.empty())
		return true;

	return false;
}

void	r_mesh_c::calcTangentSpaces()
{
	for(unsigned int i=0; i<vertexes.size(); i++)
	{
		tangents[i].clear();
		tangents[i].clear();
		tangents[i].clear();

		binormals[i].clear();
		binormals[i].clear();
		binormals[i].clear();

		normals[i].clear();
		normals[i].clear();
		normals[i].clear();
	}

	vec3_c tangent, binormal, normal;
	vec3_c face_normal;

	for(unsigned int i=0; i<indexes.size(); i += 3)
	{
		const vec3_c &v0 = vertexes[indexes[i+0]];
		const vec3_c &v1 = vertexes[indexes[i+1]];
		const vec3_c &v2 = vertexes[indexes[i+2]];

		const vec2_c &t0 = texcoords[indexes[i+0]];
		const vec2_c &t1 = texcoords[indexes[i+1]];
		const vec2_c &t2 = texcoords[indexes[i+2]];

		// compute the face normal based on vertex points
		face_normal.crossProduct(v2-v0, v1-v0);
		face_normal.normalize();

		R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2, face_normal);

		tangents[indexes[i+0]] += tangent;
		tangents[indexes[i+1]] += tangent;
		tangents[indexes[i+2]] += tangent;

		binormals[indexes[i+0]] += binormal;
		binormals[indexes[i+1]] += binormal;
		binormals[indexes[i+2]] += binormal;

		normals[indexes[i+0]] += normal;
		normals[indexes[i+1]] += normal;
		normals[indexes[i+2]] += normal;
	}

	for(unsigned int i=0; i<vertexes.size(); i++)
	{
		tangents[i].normalize();
		tangents[i].normalize();
		tangents[i].normalize();

		binormals[i].normalize();
		binormals[i].normalize();
		binormals[i].normalize();

		normals[i].normalize();
		normals[i].normalize();
		normals[i].normalize();
	}
}


void	r_mesh_c::calcTangentSpaces2()
{
	for(std::vector<vec3_c>::iterator ir = tangents.begin(); ir != tangents.end(); ++ir)
		(*ir).clear();

	for(std::vector<vec3_c>::iterator ir = binormals.begin(); ir != binormals.end(); ++ir)
		(*ir).clear();

	for(std::vector<vec3_c>::iterator ir = normals.begin(); ir != normals.end(); ++ir)
		(*ir).clear();

	vec3_c face_normal;
	vec3_c tangent, binormal, normal;
	for(std::vector<index_t>::const_iterator ir = indexes.begin(); ir != indexes.end(); ir += 3)
	{
		const vec3_c &v0 = vertexes[*(ir+0)];
		const vec3_c &v1 = vertexes[*(ir+1)];
		const vec3_c &v2 = vertexes[*(ir+2)];

		const vec2_c &t0 = texcoords[*(ir+0)];
		const vec2_c &t1 = texcoords[*(ir+1)];
		const vec2_c &t2 = texcoords[*(ir+2)];

		// compute the face normal based on vertex points
		face_normal.crossProduct(v2-v0, v1-v0);
		face_normal.normalize();

		R_CalcTangentSpace(tangent, binormal, normal, v0, v1, v2, t0, t1, t2, face_normal);

		tangents[*(ir+0)] += tangent;
		tangents[*(ir+1)] += tangent;
		tangents[*(ir+2)] += tangent;

		binormals[*(ir+0)] += binormal;
		binormals[*(ir+1)] += binormal;
		binormals[*(ir+2)] += binormal;

		normals[*(ir+0)] += normal;
		normals[*(ir+1)] += normal;
		normals[*(ir+2)] += normal;
	}

	for(std::vector<vec3_c>::iterator ir = tangents.begin(); ir != tangents.end(); ++ir)
		(*ir).normalize();

	for(std::vector<vec3_c>::iterator ir = binormals.begin(); ir != binormals.end(); ++ir)
		(*ir).normalize();

	for(std::vector<vec3_c>::iterator ir = normals.begin(); ir != normals.end(); ++ir)
		(*ir).normalize();
}

void	r_mesh_c::calcTangentSpaces3()
{
	vec3_c*	first_t = &tangents[0];
	vec3_c*	first_b = &binormals[0];
	vec3_c*	first_n = &normals[0];
	const unsigned int max = tangents.size();

	// etwas schneller als stl
	unsigned int i = 0;
	for(i = 0; i < max; i += 3)
	{
		first_t->clear();
		(++first_t)->clear();
		(++first_t)->clear();
	}

	for(i = 0; i < max; i += 3)
	{
		first_b->clear();
		(++first_b)->clear();
		(++first_b)->clear();
	}

	for(i = 0; i < max; i += 3)
	{
		first_n->clear();
		(++first_n)->clear();
		(++first_n)->clear();
	}

	vec3_c face_normal;
	vec3_c tangent, binormal, normal;
	

	vec3_c* p_vertices = &vertexes[0];
	vec2_c* p_texcoords = &texcoords[0];
	vec3_c* v0;
	vec3_c* v1;
	vec3_c* v2;
	vec2_c* t0;
	vec2_c* t1;
	vec2_c* t2;


	first_t = &tangents[0];
	first_b = &binormals[0];
	first_n = &normals[0];
#if 1
	const index_t* indices = &indexes[0];
	register index_t index; // register index_t index
	for(unsigned int i=0; i<indexes.size(); i += 3)
	{
		v0 = p_vertices + i;
		t0 = p_texcoords + i;

		v1 = p_vertices + (i + 1);
		t1 = p_texcoords + (i + 1);

		v2 = p_vertices + (i + 2);
		t2 = p_texcoords + (i + 2);

		// compute the face normal based on vertex points
		face_normal.crossProduct(*v2 - *v0, *v1 - *v0);
		face_normal.normalize();

		R_CalcTangentSpace(tangent, binormal, normal, *v0, *v1, *v2, *t0, *t1, *t2, face_normal);

/* ca 100 microsec
		*(first_t + *(indices + i)) += tangent;
		*(first_t + *(indices + (i + 1))) += tangent;
		*(first_t + *(indices + (i + 2))) += tangent;

		*(first_b + *(indices + i)) += binormal;
		*(first_b + *(indices + (i + 1))) += binormal;
		*(first_b + *(indices + (i + 2))) += binormal;

		*(first_n + *(indices + i)) += normal;
		*(first_n + *(indices + (i + 1))) += normal;
		*(first_n + *(indices + (i + 2))) += normal;
*/

// ca. 130 microsec

		index = *(indices + (i + 0));
		*(first_t + index) += tangent;
		*(first_b + index) += binormal;
		*(first_n + index) += normal;

		index = *(indices + (i + 1));
		*(first_n + index) += normal;
		*(first_b + index) += binormal;
		*(first_t + index) += tangent;

		index = *(indices + (i + 2));
		*(first_t + index) += tangent;
		*(first_b + index) += binormal;
		*(first_n + index) += normal;


// ca. 40 microsec
/*
		*(first_t + indexes[i]) += tangent;
		*(first_b + indexes[i]) += binormal;
		*(first_n + indexes[i]) += normal;

		*(first_n + indexes[i+1]) += normal;
		*(first_b + indexes[i+1]) += binormal;
		*(first_t + indexes[i+1]) += tangent;

		*(first_t + indexes[i+2]) += tangent;
		*(first_b + indexes[i+2]) += binormal;
		*(first_n + indexes[i+2]) += normal;
*/
	}
#endif

	for(i = 0; i < max; i += 3)
	{
		first_t->normalize();
		(++first_t)->normalize();
		(++first_t)->normalize();
	}

	for(i = 0; i < max; i += 3)
	{
		first_b->normalize();
		(++first_b)->normalize();
		(++first_b)->normalize();
	}

	for(i = 0; i < max; i += 3)
	{
		first_n->normalize();
		(++first_n)->normalize();
		(++first_n)->normalize();
	}
}

void	r_mesh_c::createBBoxFromVertexes()
{
	bbox.clear();

	for(std::vector<vec3_c>::const_iterator ir = vertexes.begin(); ir != vertexes.end(); ++ir)
	{
		bbox.addPoint(*ir);
	}
}

void	r_mesh_c::addVertex(const vec3_c &v)
{
	vertexes.push_back(v);
	texcoords.push_back(vec2_c());
	tangents.push_back(vec3_c());
	binormals.push_back(vec3_c());
	normals.push_back(vec3_c());
	colors.push_back(vec4_c());
}


void	r_mesh_c::addTriangle(index_t v0, index_t v1, index_t v2, uint_t num)
{
	triangles.push_back(num);

	indexes.push_back(v0);
	indexes.push_back(v1);
	indexes.push_back(v2);
	
}

void	r_mesh_c::addTriangle(index_t v0, index_t v1, index_t v2)
{
	indexes.push_back(v0);
	indexes.push_back(v1);
	indexes.push_back(v2);
}

bool	r_mesh_c::hasTriangle(uint_t num)
{
	std::vector<uint_t>::iterator ir = std::find(triangles.begin(), triangles.end(), num);
	
	if(ir != triangles.end())
		return true;
	
	return false;
}

void	r_mesh_c::addEdge(const r_wedge_t &we)
{
	edges.push_back(we);
}


void	r_mesh_c::addEdge(int v0, int v1, int plane)
{
	/*
	for(unsigned int i=0; i<edges.size(); i++)
	{
		r_winged_edge_t &we0 = edges[i];

		if(we0.indexes_vert[0] == we.indexes_vert[0] && we0.indexes_vert[1] == we.indexes_vert[1])
		{
			//Com_Error(ERR_DROP, "r_mesh_c::addEdge: facingness different between polys on edge!");
			//Com_Printf("r_mesh_c::addEdge: facingness different between polys on edge!\n");
		}

		if(we0.indexes_vert[0] == we.indexes_vert[1]  && we0.indexes_vert[1] == we.indexes_vert[0])
		{
			if(we0.indexes_tri[1] != -1)
			{
				Com_Error(ERR_DROP, "r_mesh_c::addEdge: triple edge! bad...");
			}
			we0.indexes_tri[1] = we.indexes_tri[0];	// pair the edge and return
			return;
		}
	}
	*/

	for(std::list<r_wedge_t>::iterator ir = edges_unmatched.begin(); ir != edges_unmatched.end(); ir++)
	{
		r_wedge_t &we = *ir;

		if((vertexes[we.v[0]] == vertexes[v1]) && (vertexes[we.v[1]] == vertexes[v0]))
		{
			we.plane[1] = plane;
			addEdge(we);
			edges_unmatched.erase(ir);
			return;
		}
	}

	edges_unmatched.push_back(r_wedge_t(v0, v1, plane, 0));
}

int	r_mesh_c::cap()
{
	int capped = 0;

	while(!edges_unmatched.empty())
	{
		r_wedge_t e0 = edges_unmatched.front();
		edges_unmatched.pop_front();

		capped++;

		bool unmatched = true;

		for(std::list<r_wedge_t>::iterator it = edges_unmatched.begin(); unmatched && it != edges_unmatched.end(); ++it)
		{
			if(vertexes[e0.v[0]] == vertexes[it->v[1]])
			{
				r_wedge_t e1 = *it;
				edges_unmatched.erase(it);

				planes.push_back(cplane_c(vertexes[e0.v[0]], vertexes[e1.v[0]], vertexes[e0.v[1]]));
				//triangles.push_back(Triangle(e0.v[0], e1.v[0], e0.v[1], 0, 0, 0, 0, 0));

				unsigned int plane = planes.size() - 1;
				addEdge(e1.v[0], e0.v[1], plane);

				e0.plane[1] = e1.plane[1] = plane;
				addEdge(e0);
				addEdge(e1);

				//capped++;
				break;
			}
			else if (vertexes[e0.v[1]] == vertexes[it->v[0]])
			{
				r_wedge_t e1 = *it;
				edges_unmatched.erase(it);

				planes.push_back(cplane_c(vertexes[e0.v[0]], vertexes[e1.v[1]], vertexes[e0.v[1]]));
				//triangles.push_back(Triangle(e0.v[0], e1.v[1], e0.v[1], 0, 0, 0, 0, 0));

				unsigned int plane = planes.size() - 1;
				addEdge(e0.v[0], e1.v[1], plane);

				e0.plane[1] = e1.plane[1] = plane;
				addEdge(e0);
				addEdge(e1);

				//capped++;
				break;
			}
		}
	}

	return capped;
}

void	r_mesh_c::calcEdges()
{
	// for each triangle, try to add each edge to the winged_edge vector,
	// but check first to see if it's already there
	for(unsigned int i=0; i<indexes.size(); i += 3)
	{
		planes.push_back(cplane_c(vertexes[indexes[0]], vertexes[indexes[1]], vertexes[indexes[2]]));

		addEdge(indexes[0], indexes[1], i);
		addEdge(indexes[1], indexes[2], i);
		addEdge(indexes[2], indexes[0], i);
	}


	//ri.Com_Printf("r_mesh_c::calcEdges: mesh '%s' %3i planes %3i edges %3i uedges %3i vertexes\n", name.c_str(), planes.size(), edges.size(), edges_unmatched.size(), vertexes.size());

	/*
	for(unsigned int i=0; i<edges.size(); i++)
	{
		r_winged_edge_t &we = edges[i];

		if(!we.indexes_vert[0] || !we.indexes_vert[0] || !we.indexes_tri[0] || !we.indexes_tri[0])
		{
			Com_Error(ERR_DROP, "r_mesh_c::calcEdge: mesh '%s' has bad edge");
		}
	}
	*/
}










