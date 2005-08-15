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
r_renderbuffer_c*	r_rb_lightview_stencil;

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
	if(!xglIsRenderbufferEXT(_id))
		ri.Com_Error(ERR_DROP, "r_renderbuffer_c::validate: failed");
}

void	r_renderbuffer_c::bind() const
{
	xglBindRenderbufferEXT(GL_RENDERBUFFER_EXT, _id);
}

void	r_renderbuffer_c::storage(GLenum internalformat, GLsizei width, GLsizei height) const
{
	xglRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internalformat, width, height);
}

void	r_renderbuffer_c::getParameteriv(GLenum pname, GLint *params) const
{
	//TODO
}



r_framebuffer_c::r_framebuffer_c(uint_t width, uint_t height)
{
	xglGenFramebuffersEXT(1, &_id);
	
	setRegistrationCount();

	_width	= width;
	_height	= height;
	
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
	if(!xglIsFramebufferEXT(_id))
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
			ri.Com_Error(ERR_FATAL, "r_framebuffer_c::checkStatus: Unsupported framebuffer format\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
			ri.Com_Error(ERR_FATAL, "r_framebuffer_c::checkStatus: Framebuffer incomplete, missing attachment\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DUPLICATE_ATTACHMENT_EXT:
			ri.Com_Error(ERR_FATAL, "r_framebuffer_c::checkStatus: Framebuffer incomplete, duplicate attachment\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
			ri.Com_Error(ERR_FATAL, "r_framebuffer_c::checkStatus: Framebuffer incomplete, attached images must have same dimensions\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
			ri.Com_Error(ERR_FATAL, "r_framebuffer_c::checkStatus: Framebuffer incomplete, attached images must have same format\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
			ri.Com_Error(ERR_FATAL, "r_framebuffer_c::checkStatus: Framebuffer incomplete, missing draw buffer\n");
			break;

		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
			ri.Com_Error(ERR_FATAL, "r_framebuffer_c::checkStatus: Framebuffer incomplete, missing read buffer\n");
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

void	r_framebuffer_c::attachRenderBuffer(GLenum attachment, GLuint renderbuffer) const
{
	xglFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, attachment, GL_RENDERBUFFER_EXT, renderbuffer);
}

void	r_framebuffer_c::getAttachmentParameteriv(GLenum attachment, GLenum pname, GLint *params) const
{
	//TODO
}




void	R_InitFBOs()
{
	ri.Com_Printf("------- R_InitFBOs -------\n");

	r_registrationcount = 1;

	uint_t fb_width = 256;
	uint_t fb_height = 256;

//	for(fb_width = 1; fb_width < vid.width; fb_width <<= 1);
//	for(fb_height = 1; fb_height < vid.height; fb_height <<= 1);

	r_fb_lightview = new r_framebuffer_c(fb_width, fb_height);
	r_fb_lightview->bind();

	r_image_c *image = new r_image_c(GL_TEXTURE_2D, "_lightview_color", r_fb_lightview->getWidth(), r_fb_lightview->getHeight(), IMAGE_NONE, NULL);
	image->bind();
//	xglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
	xglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image->getWidth(), image->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
//	xglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image->getWidth(), image->getHeight(), 0, GL_RGB, GL_INT, NULL);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	xglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	r_fb_lightview->attachTexture2D(GL_COLOR_ATTACHMENT0_EXT, image->getTarget(), image->getId(), 0);
	r_img_lightview_color = image;

	r_rb_lightview_depth = new r_renderbuffer_c();
	r_rb_lightview_depth->bind();
	r_rb_lightview_depth->storage(GL_DEPTH_COMPONENT24, r_fb_lightview->getWidth(), r_fb_lightview->getHeight());
	r_fb_lightview->attachRenderBuffer(GL_DEPTH_ATTACHMENT_EXT, r_rb_lightview_depth->getId());

//	r_rb_lightview_stencil = new r_renderbuffer_c();
//	r_rb_lightview_stencil->bind();
//	r_rb_lightview_stencil->storage(GL_STENCIL_INDEX_EXT, r_fb_lightview->getWidth(), r_fb_lightview->getHeight());
//	r_fb_lightview->attachRenderBuffer(GL_STENCIL_ATTACHMENT_EXT, r_rb_lightview_stencil->getId());

	r_fb_lightview->checkStatus();
	r_fb_lightview->unbind();
}

void	R_ShutdownFBOs()
{
	ri.Com_Printf("------- R_ShutdownFBOs -------\n");

	X_purge(r_renderbuffers);
	r_renderbuffers.clear();

	X_purge(r_framebuffers);
	r_framebuffers.clear();
}

