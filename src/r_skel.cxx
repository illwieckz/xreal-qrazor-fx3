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

void	r_skel_model_c::createBBox(r_entity_c *ent, cbbox_c &bbox)
{
	for(std::vector<r_skel_bone_t*>::iterator ir = _bones.begin(); ir != _bones.end(); ir++)
	{
		updateBone(ent, *ir);
	}
	
		
	int bone_index = getNumForBoneName("boundsMin");
	
	if(bone_index != -1)
		bbox._mins = _bones[bone_index]->position;
		
	
	bone_index = getNumForBoneName("boundsMax");
	
	if(bone_index != -1)
		bbox._maxs = _bones[bone_index]->position;
}

bool	r_skel_model_c::cull(r_entity_c *ent)
{
	if(ent->getShared().flags & RF_WEAPONMODEL)
		return false;
	
	if(ent->getShared().flags & RF_VIEWERMODEL)
		return (!(r_mirrorview || r_portal_view));
	
	if(R_CullBSphere(r_frustum, ent->getShared().origin, _bbox.radius()))
		return true;
	
	if((r_mirrorview || r_portal_view) && r_cull->getValue())
	{
		if(r_clipplane.distance(ent->getShared().origin) < -_bbox.radius())
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
	for(std::vector<r_skel_bone_t*>::iterator ir = _bones.begin(); ir != _bones.end(); ir++)
	{
		updateBone(cmd->getEntity(), *ir);
	}


	//
	// draw skeleton
	//
	/*
	for(std::vector<r_skel_bone_t*>::iterator ir = _bones.begin(); ir != _bones.end(); ir++)
	{
		drawBone_r(*ir);
	}
	*/
	
			
	
	//
	// update vertices
	//
	if(type != RENDER_TYPE_SHADOWING)
	{
		for(unsigned int i=0; i<mesh->vertexes.size(); i++)
		{
			mesh->vertexes[i].clear();
			
			for(std::vector<r_skel_weight_t*>::const_iterator ir = mesh->vertexweights[i].begin(); ir != mesh->vertexweights[i].end(); ir++)
			{	
				r_skel_weight_t *weight = *ir;
				r_skel_bone_t *bone = _bones[(*ir)->bone_index];
				
				vec3_c pos = bone->matrix * weight->position;
				mesh->vertexes[i] += (pos + bone->position) * weight->weight;
			}
		}
	}
	else
	{
		//TODO
	}
	
	
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
	
	createBBox(ent, _bbox);

	if(cull(ent))
	{
		if(!r_shadows->getValue())
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
		
		RB_AddCommand(ent, this, mesh, shader, NULL, NULL, -(i+1));
		
		for(std::map<int, r_light_c>::iterator ir = r_lights.begin(); ir != r_lights.end(); ++ir)
		{
			r_light_c& light = ir->second;
			
			if(light.getShared().radius_bbox.intersect(ent->getShared().origin, _bbox.radius()))
			{
				//ri.Com_Printf("r_skel_model_c::addModelToList: model has light interaction '%s'\n", getName());
				
				RB_AddCommand(ent, this, mesh, shader, &light, NULL, -(i+1));
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
	bool success = true;
	
	for(std::vector<r_skel_channel_t*>::const_iterator ir = anim->_channels.begin(); ir != anim->_channels.end(); ir++)
	{
		r_skel_channel_t *channel = *ir;
		
		int bone_index = getNumForBoneName(channel->joint);
		
		if(bone_index != -1)
		{
			// Tr3B - ignore movement of the origin bone
			// it is just a development aid for modellers and not used by the engine
			
			if(X_strequal(channel->joint.c_str(), "origin"))
			{
				if( 	channel->attribute == CHANNEL_ATTRIB_X ||
					channel->attribute == CHANNEL_ATTRIB_Y ||
					channel->attribute == CHANNEL_ATTRIB_Z
				)
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
}


void	r_skel_model_c::updateBone(const r_entity_c *ent, r_skel_bone_t *bone)
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
		mat_old = _bones[bone->parent_index]->matrix;
	else
		mat_old.identity();
		
	mat_new = mat_old;
	
	mat_new.multiplyTranslation(attr[CHANNEL_ATTRIB_X], attr[CHANNEL_ATTRIB_Y], attr[CHANNEL_ATTRIB_Z]);
	
	mat_new.multiplyRotation(attr[CHANNEL_ATTRIB_PITCH], attr[CHANNEL_ATTRIB_YAW], attr[CHANNEL_ATTRIB_ROLL]);
	
//	mat_lerp.lerp(mat_old, mat_new, ent->getShared().backlerp);
	
	bone->matrix = mat_new;
	bone->position.set(mat_new[0][3], mat_new[1][3], mat_new[2][3]);
}

void	r_skel_model_c::drawBone(r_skel_bone_t *bone)
{
	xglColor4fv(color_white);
				
	if(bone->parent_index != -1)
	{
		xglBegin(GL_LINES);
			
		xglVertex3fv(_bones[bone->parent_index]->position);	
		xglVertex3fv(bone->position);
			
		xglEnd();
	}
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





