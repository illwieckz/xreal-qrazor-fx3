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


/// includes ===================================================================
// system -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include "r_local.h"


r_skel_model_c::r_skel_model_c(const std::string &name, byte *buffer, uint_t buffer_size)
:r_model_c(name, buffer, buffer_size, MOD_SKEL)
{
	//ri.Com_Printf ("r_skel_model_c::ctor: %s\n", name);
}

r_skel_model_c::~r_skel_model_c()
{
	/*
	for(std::vector<r_skel_frame_t*>::const_iterator ir = _frames.begin(); ir != _frames.end(); ir++)
	{
		for(std::vector<r_skel_bone_frame_t*>::iterator ir2 = (*ir)->boneframes.begin(); ir2 != (*ir)->boneframes.end(); ir2++)
		{
			delete *ir2;
		}
		(*ir)->boneframes.clear();
	}
	_frames.clear();
	*/
	
	//TODO clear extra bone information
}

const aabb_c	r_skel_model_c::createAABB(r_entity_c *ent)
{
	const r_skel_animation_c* anim = ent->getAnimation();
	
	if(!anim)
		return aabb_c();

	// get frames
	if((ent->getShared().frame < 0) || (ent->getShared().frame >= (int)anim->_frames.size()))
	{
		ri.Com_Printf("r_skel_model_c::createAABB: animation '%s' has no such frame %d\n", anim->getName(), ent->getShared().frame);
	}
	
	if((ent->getShared().frame_old < 0) || (ent->getShared().frame_old >= (int)anim->_frames.size()))
	{
		ri.Com_Printf ("r_skel_model_c::createAABB: animation '%s' has no such oldframe %d\n", anim->getName(), ent->getShared().frame_old);
	}

	r_skel_frame_t* frame		= anim->_frames[X_bound(0, ent->getShared().frame, (int)anim->_frames.size()-1)];
	r_skel_frame_t* frame_old	= anim->_frames[X_bound(0, ent->getShared().frame_old, (int)anim->_frames.size()-1)];


	// compute axially aligned mins and maxs
	aabb_c aabb;
	aabb.clear();
	if(frame == frame_old)
	{
		aabb	= frame->bbox;
	}
	else
	{
		aabb = frame_old->bbox;
		aabb.mergeWith(frame->bbox);	
	}

	return aabb;
}

bool	r_skel_model_c::cull(r_entity_c *ent)
{
	if(ent->getShared().flags & RF_WEAPONMODEL)
		return false;
	
	if(ent->getShared().flags & RF_VIEWERMODEL)
		return (!(r_mirrorview || r_portal_view));
	
	if(r_frustum.cull(ent->getAABB()))
		return true;
	
	if((r_mirrorview || r_portal_view) && r_cull->getValue())
	{
		if(r_clipplane.distance(ent->getShared().origin) < -ent->getAABB().radius())
			return true;
	}
	
	return false;
}


void	r_skel_model_c::drawFrameLerp(const r_command_t *cmd, r_render_type_e type)
{
	//ri.Com_Printf("r_alias_model_c::drawFrameLerp: model '%s'\n", getName());
	
	r_skel_mesh_c*	mesh = NULL;
	
	if(!cmd)
		return;

	if(!cmd->getEntity())
		return;
		
	if(!(mesh = (r_skel_mesh_c*)cmd->getEntityMesh()))
		return;
	
	//if(!r_lerpmodels->getValue())
	//	cmd->getEntity()->getShared().backlerp = 0;
	
	
	//
	// update skeleton
	//
#if 1
	updateBones(cmd->getEntity());
#endif

	//
	// draw skeleton
	//
#if 0
	drawBones();
#endif
	
	//
	// update vertices
	//
#if 1
	if(type != RENDER_TYPE_SHADOWING)
	{
		for(unsigned int i=0; i<mesh->vertexes.size(); i++)
		{
			mesh->vertexes[i].clear();
			
			for(std::vector<r_skel_weight_t*>::const_iterator ir = mesh->vertexweights[i].begin(); ir != mesh->vertexweights[i].end(); ++ir)
			{	
				const r_skel_weight_t *weight = *ir;
				r_skel_bone_t *bone = _bones[weight->bone_index];
				
				mesh->vertexes[i] += (bone->origin + (bone->quat * weight->position)) * weight->weight;
			}
		}
	}
	else
	{
		//TODO
	}
#endif
	
	
	//
	// update tangent spaces
	//
	mesh->calcTangentSpaces();
		
	
	//
	// draw lerped mesh
	//
	RB_RenderCommand(cmd, type);
}


void	r_skel_model_c::addModelToList(r_entity_c *ent)
{
	if((ent->getShared().flags & RF_WEAPONMODEL) && (r_lefthand->getInteger() == 2))
			return;
			
	//ri.Com_Printf("r_skel_model_c::addModelToList: model '%s'\n", getName());
	
	if(ent->isVisible() && cull(ent))
	{
		c_entities--;
		return;
	}
		//TODO
	//else
		//r_entvisframe[e->number][(r_
		
	
	
	//
	// add meshes to the mesh buffer lists with appropiate skins
	//
	for(unsigned i=0; i<_meshes.size(); i++)
	{
		r_mesh_c*	mesh = _meshes[i];
		r_shader_c*	shader = NULL;
		
		if(ent->getShared().custom_shader != -1)
		{
			shader = R_GetShaderByNum(ent->getShared().custom_shader);
		}
		else if(ent->getShared().custom_skin != -1)
		{
			r_model_skin_c *skin = R_GetSkinByNum(ent->getShared().custom_skin);
			
			shader = skin->getShader(mesh->name);
		}
		else if(i >= 0 && i < _shaders.size())
		{
			shader = _shaders[i]->getShader();
		}
		else
		{
			ri.Com_Error(ERR_DROP, "r_skel_model_c::addModelToList: no way to create command");
			continue;
		}
		
		if(!r_showinvisible->getValue() && shader->hasFlags(SHADER_NODRAW))
			continue;
			
		if(r_envmap && shader->hasFlags(SHADER_NOENVMAP))
			continue;
		
		RB_AddCommand(ent, this, mesh, shader, NULL, NULL, -(i+1), r_origin.distance(ent->getShared().origin));
		
		for(std::vector<std::vector<r_light_c> >::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			std::vector<r_light_c>& lights = *ir;
			
			for(std::vector<r_light_c>::iterator ir = lights.begin(); ir != lights.end(); ++ir)
			{
				r_light_c& light = *ir;
			
				if(!light.isVisible())
					continue;
			
				// FIXME
				if(light.getShared().radius_aabb.intersect(ent->getShared().origin, mesh->bbox.radius()))
					RB_AddCommand(ent, this, mesh, shader, &light, NULL, -(i+1), 0);
			}
		}
	}
}

void 	r_skel_model_c::draw(const r_command_t *cmd, r_render_type_e type)
{
	//ri.Com_Printf("r_skel_model_c::draw: model '%s'\n", getName());
	
	if(!cmd)
		return;
		
	if((cmd->getEntity()->getShared().flags & RF_WEAPONMODEL) && (type == RENDER_TYPE_SHADOWING))
		return;
		
	if(cmd->getEntity()->getShared().flags & RF_DEPTHHACK) // hack the depth range to prevent view model from poking into walls
		xglDepthRange(r_depthmin, r_depthmin + 0.3*(r_depthmax-r_depthmin));

	if((cmd->getEntity()->getShared().flags & RF_WEAPONMODEL) && (r_lefthand->getInteger() == 1))
	{
		cmd->getEntity()->setupTransformLeftHanded();
		xglFrontFace(GL_CW);
	}
	else
	{
		cmd->getEntity()->setupTransform();
	}
	
	RB_SetupModelviewMatrix(cmd->getEntity()->getTransform());
	
	drawFrameLerp(cmd, type);
		
	if((cmd->getEntity()->getShared().flags & RF_WEAPONMODEL) && (r_lefthand->getInteger() == 1)) 
		xglFrontFace(GL_CCW);
	
	if(cmd->getEntity()->getShared().flags & RF_DEPTHHACK)
		xglDepthRange(r_depthmin, r_depthmax);
}

bool	r_skel_model_c::setupTag(r_tag_t &tag, const r_entity_t &ent, const std::string &name)
{
#if 0
	//r_skel_frame_t*		frame;
	//r_skel_frame_t*		frame_old;
	
	
	//r_skel_bone_frame_t*	boneframe;
	//r_skel_bone_frame_t*	boneframe_old;
	
	
	r_skel_tag_t*		tag;
	r_skel_bone_t*		bone;
	
	//ri.Com_Printf("r_skel_model_c::setupTag: model '%s'\n", getName());

	//
	// get frames
	//
	/*
	if(ent.frame < 0 || ent.frame >= (int)_frames.size())
	{
		ri.Com_Printf("r_skel_model_c::setupTag: model %s has no such frame %d\n", getName(), ent.frame);
		return false;
	}
	
	if(ent.frame_old < 0 || ent.frame_old >= (int)_frames.size())
	{
		ri.Com_Printf("r_skel_model_c::setupTag: model %s has no such frame %d\n", getName(), ent.frame_old);
		return false;
	}
	
	
	frame = _frames[ent.frame];
	frame_old = _frames[ent.frame_old];
	*/	
	
	//
	// get tag
	//
	unsigned int tag_num;
	for(tag_num=0; tag_num<_tags.size(); tag_num++)
	{
		if(X_strcaseequal(_tags[tag_num]->name.c_str(), name.c_str()))
			break;
	}
	
	if(tag_num < 0 || tag_num >= _tags.size())
	{
		ri.Com_Printf("r_skel_model_c::setupTag: model '%s' has no such tag '%s' %i %i\n", getName(), name.c_str(), _tags.size(), tag_num);
		return false;
	}
		
	tag = _tags[tag_num];

	
	//
	// update bones
	//
	updateBone(&ent, tag->bone_index);
	
	bone = _bones[tag->bone_index];
	
	
	//
	// get orientation and origin
	//	
	orient.origin = bone->origin;
	orient.quat = bone->rot_quat;
	

	return true;
#endif

	return false;
}


bool	r_skel_model_c::setupAnimation(r_skel_animation_c *anim)
{
#if 0
	bool success = true;
	
	for(std::vector<r_skel_channel_t*>::const_iterator ir = anim->_channels.begin(); ir != anim->_channels.end(); ++ir)
	{
		r_skel_channel_t *channel = *ir;
		
		int bone_index = getNumForBoneName(channel->joint);
		
		if(bone_index != -1)
		{
			// Tr3B - ignore movement of the origin bone
			// it is just a development aid for modellers and not used by the engine
			
			if(X_strequal(channel->joint.c_str(), "origin"))
			{
				if(channel->attributes & (CHANNEL_BIT_X | CHANNEL_BIT_Y | CHANNEL_BIT_Z))
					continue;
			}
			
			_bones[bone_index]->channels[channel->attribute] = channel;
		}
		else
		{
			ri.Com_Printf("r_skel_model_c::setupAnimation: model '%s' has no bone '%s'\n", getName(), channel->joint.c_str());
			success = false;
		}
	}
	
	return success;
#else
	return false;
#endif
}

/*
void	r_skel_model_c::updateBones(const r_entity_c *ent)
{
	//ri.Com_Printf("r_skel_model_c::updateBone: %s\n", bone->name.c_str());
	
	float	attr[6];
	attr[0] = attr[1] = attr[2] = attr[3] = attr[4] = attr[5] = 0;
		
	for(int i=0; i<6; i++)
	{
		r_skel_channel_t* channel = bone->channels[i];
		
		if(!channel)
			continue;
			
//		int frame = (int)(X_bound(channel->time_start, ent->getShared().time, channel->time_end) / channel->time_end) * (channel->keys.size()-1);
						
//		attr[i] = channel->keys[X_bound(0, frame, (int)channel->keys.size()-1)];

//		attr[i] = channel->keys[X_bound(X_max(0, channel->range[0]), ent->getShared().frame, X_min((int)channel->keys.size()-1, channel->range[1]))];
		
//		attr[i] = channel->keys[ent->getShared().frame % channel->keys.size()];
		
		attr[i] = channel->keys[X_bound(0, ent->getShared().frame, (int)channel->keys.size()-1)];
		
//		attr[i] = channel->keys[X_bound(channel->range[0], ent->getShared().frame, channel->range[1])];
	}
		
	matrix_c mat_old;
	matrix_c mat_new;
//	matrix_c mat_lerp;
	
	if(bone->parent_index != -1)
	{
		mat_old = _bones[bone->parent_index]->matrix_original;
	}
	else
		mat_old.identity();
		
	mat_new = mat_old;
	
	mat_new.multiplyTranslation(attr[CHANNEL_ATTRIB_X], attr[CHANNEL_ATTRIB_Y], attr[CHANNEL_ATTRIB_Z]);
	
	mat_new.multiplyRotation(attr[CHANNEL_ATTRIB_PITCH], attr[CHANNEL_ATTRIB_YAW], attr[CHANNEL_ATTRIB_ROLL]);
	
//	mat_lerp.lerp(mat_old, mat_new, ent->getShared().backlerp);
	
	bone->matrix_dynamic = mat_new;
	bone->position.set(mat_new[0][3], mat_new[1][3], mat_new[2][3]);
*/


void	r_skel_model_c::updateBones(const r_entity_c *ent)
{
	for(uint_t i=0; i<_bones.size(); i++)
	{
		updateBones_r(ent, i);
	}
}


void	r_skel_model_c::updateBones_r(const r_entity_c *ent, int bone_index)
{
	const r_skel_animation_c* anim = ent->getAnimation();
	
	if(_bones.size() != anim->_channels.size())
			ri.Com_Error(ERR_DROP, "r_skel_model_c::updateBones: bones number != animated channels number");		
	
	r_skel_bone_t* bone = NULL;
	try
	{
		bone = _bones.at(bone_index);
	}
	catch(...)
	{
		ri.Com_Error(ERR_DROP, "r_skel_model_c::updateBones: exception occured while getting bone using index %i", bone_index);
	}
	
	if(!anim)
	{
		// take original values
		bone->origin = bone->default_origin;
		bone->quat = bone->default_quat;
	}
	else
	{
		r_skel_channel_t* channel = NULL;
		try
		{
			channel = anim->_channels.at(bone_index);
		}
		catch(...)
		{
			ri.Com_Error(ERR_DROP, "r_skel_model_c::updateBones: exception occured while getting channel using index %i", bone_index);
		}
	
		if(bone->parent_index != channel->parent_index)
			ri.Com_Error(ERR_DROP, "r_skel_model_c::updateBones: bone parent index != channel parent index");
			
		
		//
		// set baseframe values
		//
		vec3_c		origin_old = channel->origin;
		vec3_c		origin_new = channel->origin;
		vec3_c		origin_lerp = channel->origin;
			
		quaternion_c	quat_old = channel->quat;
		quaternion_c	quat_new = channel->quat;
		quaternion_c	quat_lerp = channel->quat;
		
		//
		// get frames
		//
		r_skel_frame_t* frame_old = NULL;
		r_skel_frame_t* frame_new = NULL;
		
		if(ent->getShared().flags & RF_AUTOANIM)
		{
			frame_old = anim->_frames[ent->getShared().frame % (int)anim->_frames.size()];
			frame_new = anim->_frames[ent->getShared().frame % (int)anim->_frames.size()];
		}
		else
		{
			if((ent->getShared().frame_old < 0) || (ent->getShared().frame_old >= (int)anim->_frames.size()))
			{
				ri.Com_Printf ("r_skel_model_c::updateBones: animation %s has no such oldframe %d\n", anim->getName(), ent->getShared().frame_old);
			}
		
			if((ent->getShared().frame < 0) || (ent->getShared().frame >= (int)anim->_frames.size()))
			{
				ri.Com_Printf("r_skel_model_c::updateBones: animation %s has no such frame %d\n", anim->getName(), ent->getShared().frame);
			}
		
			frame_old = anim->_frames[X_bound(0, ent->getShared().frame_old, (int)anim->_frames.size()-1)];
			frame_new = anim->_frames[X_bound(0, ent->getShared().frame, (int)anim->_frames.size()-1)];
		}
				
		//
		// update components
		//
		
		// update translation bits
		int components_applied = 0;
		if(channel->components_bits & COMPONENT_BIT_TX)
		{
			origin_old[0] = frame_old->components[channel->components_offset+components_applied];
			origin_new[0] = frame_new->components[channel->components_offset+components_applied];
			components_applied++;
		}	
		if(channel->components_bits & COMPONENT_BIT_TY)
		{
			origin_old[1] = frame_old->components[channel->components_offset+components_applied];
		origin_new[1] = frame_new->components[channel->components_offset+components_applied];
			components_applied++;
		}	
		if(channel->components_bits & COMPONENT_BIT_TZ)
		{
			origin_old[2] = frame_old->components[channel->components_offset+components_applied];
			origin_new[2] = frame_new->components[channel->components_offset+components_applied];
			components_applied++;
		}
			
		// update quaternion rotation bits
		if(channel->components_bits & COMPONENT_BIT_QX)
		{
			((vec_t*)quat_old)[0] = frame_old->components[channel->components_offset+components_applied];
			((vec_t*)quat_new)[0] = frame_new->components[channel->components_offset+components_applied];
			components_applied++;
		}	
		if(channel->components_bits & COMPONENT_BIT_QY)
		{
			((vec_t*)quat_old)[1] = frame_old->components[channel->components_offset+components_applied];
			((vec_t*)quat_new)[1] = frame_new->components[channel->components_offset+components_applied];
			components_applied++;
		}				
		if(channel->components_bits & COMPONENT_BIT_QZ)
		{
			((vec_t*)quat_old)[2] = frame_old->components[channel->components_offset+components_applied];
			((vec_t*)quat_new)[2] = frame_new->components[channel->components_offset+components_applied];
		}
			
		quat_old.calcW();
		quat_old.normalize();
			
		quat_new.calcW();
		quat_new.normalize();
		
		//
		// lerp between old data and new data
		//
		origin_lerp.lerp(origin_old, origin_new, r_newrefdef.lerp);
		
		quat_lerp.slerp(quat_old, quat_new, r_newrefdef.lerp);
		quat_lerp.normalize();
		
		//
		// calculate bone
		//
		if(bone->parent_index < 0)
		{
			bone->origin = vec3_origin;	//origin_lerp;
			bone->quat = quat_lerp;
		}
		else
		{
			r_skel_bone_t* parent = NULL;
			try
			{
				parent = _bones.at(bone->parent_index);
			}
			catch(...)
			{
				ri.Com_Error(ERR_DROP, "r_skel_model_c::updateBones: exception occured while getting parent bone");
			}
			
			bone->origin = parent->origin + (parent->quat * origin_lerp);
			bone->quat = parent->quat * quat_lerp;
			
		}
		
	}
	
#if 0
	for(uint_t i=0; i<bone->childrens.size(); i++)
	{
		updateBones_r(ent, bone->childrens[i]->index);
	}
#endif
}

void	r_skel_model_c::drawBones()
{
	for(std::vector<r_skel_bone_t*>::iterator ir = _bones.begin(); ir != _bones.end(); ++ir)
	{
		r_skel_bone_t* bone = *ir;
		
		if(bone->parent_index >= 0)
		{
			xglBegin(GL_LINES);
			xglColor4fv(color_white);
			xglVertex3fv(_bones[bone->parent_index]->origin);
			xglVertex3fv(bone->origin);
				
			vec3_c mid = bone->origin;// - _bones[bone->parent_index]->position;
			//mid *= 0.5f;
			//mid += _bones[bone->parent_index]->position;
		
			vec3_c vf(false), vr(false), vu(false);
		
			bone->quat.toVectorsFRU(vf, vr, vu);
		
			vf += mid;
			vr += mid;
			vu += mid;
			
			//vf *= 4;
			//vr *= 4;
			//vu *= 4;
			
			xglColor4fv(color_red);
			xglVertex3fv(mid);
			xglVertex3fv(vf);
				
			xglColor4fv(color_green);
			xglVertex3fv(mid);
			xglVertex3fv(vr);
			
			xglColor4fv(color_blue);
			xglVertex3fv(mid);
			xglVertex3fv(vu);
			
			xglEnd();
		}
	}
}

void	r_skel_model_c::addBone(r_skel_bone_t *bone)
{
	bone->index = _bones.size();

	if(bone->parent_index >= 0)
	{
		r_skel_bone_t* parent = NULL;
		
		try
		{
			parent = _bones.at(bone->parent_index);
		}
		catch(...)
		{
			ri.Com_Error(ERR_DROP, "r_skel_model_c::addBone: exception occured while getting parent for bone '%s'", bone->name.c_str());
		}
			
		parent->childrens.push_back(bone);
	}
	
	_bones.push_back(bone);
}


int	r_skel_model_c::getNumForBoneName(const std::string &name)
{
	for(unsigned int i=0; i<_bones.size(); i++)
	{	
		if(X_strcaseequal(_bones[i]->name.c_str(), name.c_str()))
			return i;
	}
	
	//ri.Com_Error(ERR_DROP, "r_skel_model_c::getNumForBoneName: model '%s' has no such bone '%s'\n", getName(), name.c_str());
	return -1;
}





