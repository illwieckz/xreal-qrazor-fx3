/// ============================================================================
/*
Copyright (C) 2005 Robert Beckebans <trebor_7@users.sourceforge.net>
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
// shared -------------------------------------------------------------------
// qrazor-fx ----------------------------------------------------------------
#include 	"r_local.h"

// xreal --------------------------------------------------------------------


r_framebuffer_c*	r_fb_lightview;
r_renderbuffer_c*	r_rb_lightview_color;
r_renderbuffer_c*	r_rb_lightview_depth;

std::vector<r_renderbuffer_c*>	r_renderbuffers;
std::vector<r_framebuffer_c*>	r_framebuffers;

r_renderbuffer_c::r_renderbuffer_c()
{
	xglGenRenderbuffersEXT(1, &_id);
	
	setRegistrationCount();
	
	// find free slot
	std::vector<r_renderbuffer_c*>::iterator ir = std::find(r_renderbuffers.begin(), r_renderbuffers.end(), static_cast<r_renderbuffer_c*>(NULL));
	
	if(ir != r_renderbuffers.end())
		*ir = this;
	else
		r_renderbuffers.push_back(this);
}

r_renderbuffer_c::~r_renderbuffer_c()
{
	if(_id != 0)
	{
		xglDeleteRenderbuffersEXT(1, &_id);
	}
}

void	r_renderbuffer_c::validate() const
{
	//TODO
}

void	r_renderbuffer_c::bind() const
{
	//TODO
}

void	r_renderbuffer_c::getParameteriv(GLenum pname, GLint *params) const
{
	//TODO
}



r_framebuffer_c::r_framebuffer_c()
{
	xglGenFramebuffersEXT(1, &_id);
	
	setRegistrationCount();
	
	// find free slot
	std::vector<r_framebuffer_c*>::iterator ir = std::find(r_framebuffers.begin(), r_framebuffers.end(), static_cast<r_framebuffer_c*>(NULL));
	
	if(ir != r_framebuffers.end())
		*ir = this;
	else
		r_framebuffers.push_back(this);
}

r_framebuffer_c::~r_framebuffer_c()
{
	if(_id != 0)
	{
		xglDeleteFramebuffersEXT(1, &_id);
	}
}

void	r_framebuffer_c::validate() const
{
	if(!xglIsRenderbufferEXT(_id))
		ri.Com_Error(ERR_DROP, "r_framebuffer_c::validate: failed");
}

void	r_framebuffer_c::bind() const
{
	xglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, _id);
}

void	r_framebuffer_c::unbind() const
{
	xglBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

void	r_framebuffer_c::checkStatus() const
{
	GLenum status;
	status = (GLenum) xglCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	
	switch(status)
	{
		case GL_FRAMEBUFFER_COMPLETE_EXT:
			break;

		case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
			ri.Com_Printf("Unsupported framebuffer format\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			ri.Com_Printf("Framebuffer incomplete, missing attachment\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
			ri.Com_Printf("Framebuffer incomplete, duplicate attachment\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			ri.Com_Printf("Framebuffer incomplete, attached images must have same dimensions\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			ri.Com_Printf("Framebuffer incomplete, attached images must have same format\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			ri.Com_Printf("Framebuffer incomplete, missing draw buffer\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			ri.Com_Printf("Framebuffer incomplete, missing read buffer\n");
			break;

		default:
			assert(0);
	}
}

void	r_framebuffer_c::attachTexture1D(GLenum attachment, GLenum textarget, GLuint texture, GLint level) const
{
	xglFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT, attachment, textarget, texture, level);
}

void	r_framebuffer_c::attachTexture2D(GLenum attachment, GLenum textarget, GLuint texture, GLint level) const
{
	xglFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, attachment, textarget, texture, level);
}

void	r_framebuffer_c::attachTexture3D(GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset) const
{
	xglFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT, attachment, textarget, texture, level, zoffset);
}

void	r_framebuffer_c::renderBuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) const
{
	//TODO
}

void	r_framebuffer_c::getAttachmentParameteriv(GLenum attachment, GLenum pname, GLint *params) const
{
	//TODO
}




void	R_InitFBOs()
{
	ri.Com_Printf("------- R_InitFBOs -------\n");

	r_registrationcount = 1;

	r_fb_lightview = new r_framebuffer_c();
	
//	r_rb_lightview_color = new r_renderbuffer_c();
//	r_rb_lightview_depth = new r_renderbuffer_c();

	r_fb_lightview->checkStatus();
}

void	R_ShutdownFBOs()
{
	ri.Com_Printf("------- R_ShutdownFBOs -------\n");

	X_purge(r_renderbuffers);
	r_renderbuffers.clear();

	X_purge(r_framebuffers);
	r_framebuffers.clear();
}

