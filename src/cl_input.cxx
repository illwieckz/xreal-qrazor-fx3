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
// qrazor-fx ----------------------------------------------------------------
#include "cl_local.h"

#include "cmd.h"
#include "cvar.h"
#include "sys.h"




/*
===============================================================================

KEY BUTTONS

Continuous button event tracking is complicated by the fact that two different
input sources (say, mouse button 1 and the control key) can both press the
same button, but the button should only be released when both of the
pressing key have been released.

When a key event issues a button command (+forward, +attack, etc), it appends
its key number as a parameter to the command so it can be matched up with
the release.

state bit 0 is the current state of the key
state bit 1 is edge triggered on the up to down transition
state bit 2 is edge triggered on the down to up transition


Key_Event (int key, qboolean down, unsigned time);

  +mlook src time

===============================================================================
*/


kbutton_t	in_left, in_right, in_forward, in_back;
kbutton_t	in_lookup, in_lookdown, in_moveleft, in_moveright;
kbutton_t	in_strafe, in_speed, in_use, in_attack1, in_attack2, in_attack3;
kbutton_t	in_up, in_down;

static int	in_mx;
static int	in_my;

static int	in_mx_old;
static int	in_my_old;


static cvar_t*	cl_upspeed;
static cvar_t*	cl_forwardspeed;
static cvar_t*	cl_sidespeed;

static cvar_t*	cl_yawspeed;
static cvar_t*	cl_pitchspeed;

static cvar_t*	cl_anglespeedkey;
static cvar_t*	cl_run;

static cvar_t*	m_sensitivity;
static cvar_t*	m_filter;
static cvar_t*	m_pitch;
static cvar_t*	m_yaw;
static cvar_t*	m_forward;
static cvar_t*	m_side;



static void	KeyDown(kbutton_t *b)
{
	int		k;
	const char	*c;
	
	c = Cmd_Argv(1);
	if(c[0])
		k = atoi(c);
	else
		k = -1;		// typed manually at the console for continuous down

	if(k == b->down[0] || k == b->down[1])
		return;		// repeating key
	
	if(!b->down[0])
		b->down[0] = k;	
	else if(!b->down[1])
		b->down[1] = k;
	else
	{
		Com_Printf("Three keys down for a button!\n");
		return;
	}
	
	if(b->state & 1)
		return;		// still down

	b->state |= 1 + 2;	// down + impulse down
}

static void	KeyUp(kbutton_t *b)
{
	int		k;
	const char	*c;

	c = Cmd_Argv(1);
	if(c[0])
		k = atoi(c);
	else
	{	// typed manually at the console, assume for unsticking, so clear all
		b->down[0] = b->down[1] = 0;
		b->state = 4;	// impulse up
		return;
	}

	if(b->down[0] == k)
		b->down[0] = 0;
	else if(b->down[1] == k)
		b->down[1] = 0;
	else
		return;		// key up without coresponding down (menu pass through)
	
	if(b->down[0] || b->down[1])
		return;		// some other key is still holding it down

	if(!(b->state & 1))
		return;		// still up (this should not happen)

	b->state &= ~1;		// now up
	b->state |= 4; 		// impulse up
}

void IN_UpDown() 	{KeyDown(&in_up);}
void IN_UpUp()		{KeyUp(&in_up);}
void IN_DownDown()	{KeyDown(&in_down);}
void IN_DownUp()	{KeyUp(&in_down);}
void IN_LeftDown()	{KeyDown(&in_left);}
void IN_LeftUp()	{KeyUp(&in_left);}
void IN_RightDown()	{KeyDown(&in_right);}
void IN_RightUp()	{KeyUp(&in_right);}
void IN_ForwardDown()	{KeyDown(&in_forward);}
void IN_ForwardUp()	{KeyUp(&in_forward);}
void IN_BackDown()	{KeyDown(&in_back);}
void IN_BackUp()	{KeyUp(&in_back);}
void IN_LookupDown()	{KeyDown(&in_lookup);}
void IN_LookupUp()	{KeyUp(&in_lookup);}
void IN_LookdownDown()	{KeyDown(&in_lookdown);}
void IN_LookdownUp()	{KeyUp(&in_lookdown);}
void IN_MoveleftDown()	{KeyDown(&in_moveleft);}
void IN_MoveleftUp()	{KeyUp(&in_moveleft);}
void IN_MoverightDown()	{KeyDown(&in_moveright);}
void IN_MoverightUp()	{KeyUp(&in_moveright);}

void IN_SpeedDown()	{KeyDown(&in_speed);}
void IN_SpeedUp()	{KeyUp(&in_speed);}
void IN_StrafeDown()	{KeyDown(&in_strafe);}
void IN_StrafeUp()	{KeyUp(&in_strafe);}

void IN_Attack1Down()	{KeyDown(&in_attack1);}
void IN_Attack1Up()	{KeyUp(&in_attack1);}
void IN_Attack2Down()	{KeyDown(&in_attack2);}
void IN_Attack2Up()	{KeyUp(&in_attack2);}

void IN_UseDown()	{KeyDown(&in_use);}
void IN_UseUp()		{KeyUp(&in_use);}


/*
===============
CL_KeyState

Returns 0.25 if a key was pressed and released during the frame,
0.5 if it was pressed and held
0 if held then released, and
1.0 if held for the entire time
===============
*/
static float	CL_KeyState(kbutton_t *key)
{
	float	val;
	bool	impulsedown, impulseup, down;
	
	impulsedown	= ((key->state & 2) != 0);
	impulseup	= ((key->state & 4) != 0);
	down		= ((key->state & 1) != 0);

	key->state &= 1;	// clear impulses
	val = 0;

	if(impulsedown && !impulseup)
	{
		val = down ? 0.5 : 0;
	}
	else if(impulseup && !impulsedown)
	{
		val = 0;
	}
	else if(!impulsedown && !impulseup)
	{
		val = down ? 1 : 0;
	}
	else if(impulsedown && impulseup)
	{
		val = down ? 0.75 : 0.25;
	}

	return val;
}



/*
================
CL_AdjustAngles

Moves the local angle positions
================
*/
static void	CL_AdjustAngles()
{
	float	speed;
	
	if(in_speed.state & 1)
		speed = cls.frametime * 0.001 * cl_anglespeedkey->getValue();
	else
		speed = cls.frametime * 0.001;

	if(!(in_strafe.state & 1))
	{
		cl.view_angles[YAW] -= speed*cl_yawspeed->getValue() * CL_KeyState(&in_right);
		cl.view_angles[YAW] += speed*cl_yawspeed->getValue() * CL_KeyState(&in_left);
	}
	
	cl.view_angles[PITCH] -= speed*cl_pitchspeed->getValue() * CL_KeyState(&in_lookup);
	cl.view_angles[PITCH] += speed*cl_pitchspeed->getValue() * CL_KeyState(&in_lookdown);
}

/*
================
CL_BaseMove

Send the intended movement message to the server
================
*/
static void	CL_BaseMove(usercmd_t &cmd)
{	
	CL_AdjustAngles();
	
	cmd.clear();
	
	cmd.angles = cl.view_angles;
	
	if(in_strafe.state & 1)
	{
		cmd.sidemove += cl_sidespeed->getValue() * CL_KeyState(&in_right);
		cmd.sidemove -= cl_sidespeed->getValue() * CL_KeyState(&in_left);
	}

	cmd.sidemove += /*cl_sidespeed->getValue()*/ 127 * CL_KeyState(&in_moveright);
	cmd.sidemove -= /*cl_sidespeed->getValue()*/ 127 * CL_KeyState(&in_moveleft);

	cmd.upmove += /*cl_upspeed->getValue()*/ 127 * CL_KeyState(&in_up);
	cmd.upmove -= /*cl_upspeed->getValue()*/ 127 * CL_KeyState(&in_down);

	cmd.forwardmove += /*cl_forwardspeed->getValue()*/ 127 * CL_KeyState(&in_forward);
	cmd.forwardmove -= /*cl_forwardspeed->getValue()*/ 127 * CL_KeyState(&in_back);

	// adjust for speed key / running
	if((in_speed.state & 1) ^ cl_run->getInteger())
	{
		//cmd.forwardmove *= 2;
		//cmd.sidemove *= 2;
		//cmd.upmove *= 2;
		
		cmd.buttons &= ~BUTTON_WALK;
	}
	else
	{
		cmd.buttons |= BUTTON_WALK;
	}

	X_clamp(cmd.forwardmove, -127, 127);
	X_clamp(cmd.sidemove, -127, 127);
	X_clamp(cmd.upmove, -127, 127);
}

static void	CL_ClampPitch()
{
	float	pitch;

	pitch = cl.frame.playerstate.pmove.delta_angles[PITCH];
	
	if(pitch > 180)
		pitch -= 360;

	if(cl.view_angles[PITCH] + pitch < -360)
		cl.view_angles[PITCH] += 360; // wrapped
		
	if(cl.view_angles[PITCH] + pitch > 360)
		cl.view_angles[PITCH] -= 360; // wrapped

	if(cl.view_angles[PITCH] + pitch > 89)
		cl.view_angles[PITCH] = 89 - pitch;
		
	if(cl.view_angles[PITCH] + pitch < -89)
		cl.view_angles[PITCH] = -89 - pitch;
}

static void	CL_MouseMove(usercmd_t &cmd)
{
	if(m_filter->getInteger())
	{
		in_mx = (int)((in_mx_old + in_mx) * 0.5);
		in_my = (int)((in_my_old + in_my) * 0.5);
	}

	in_mx_old = in_mx;
	in_my_old = in_my;

	in_mx = (int)(in_mx * m_sensitivity->getValue());
	in_my = (int)(in_my * m_sensitivity->getValue());

	cl.view_angles[YAW]	-= m_yaw->getValue() * in_mx;
	cl.view_angles[PITCH]	+= m_pitch->getValue() * in_my;

	in_mx = in_my = 0;

	CL_ClampPitch();
	
	cmd.angles = cl.view_angles;
}

static void	CL_FinishMove(usercmd_t &cmd)
{
	// figure button bits	
	if(in_attack1.state & 3)
		cmd.buttons |= BUTTON_ATTACK;
	in_attack1.state &= ~2;
	
	if(in_attack2.state & 3)
		cmd.buttons |= BUTTON_ATTACK2;
	in_attack2.state &= ~2;
	
	if(in_use.state & 3)
		cmd.buttons |= BUTTON_USE;
	in_use.state &= ~2;
	
	if(in_use.state & 3)
		cmd.buttons |= BUTTON_WALK;
	in_use.state &= ~2;

	if(anykeydown && cls.key_dest == KEY_GAME)
		cmd.buttons |= BUTTON_ANY;

	// send milliseconds of time to apply the move
#if 0
	cmd.msec = X_bound(0, cls.frametime, 200);
#else
	static int extramsec = 0;
	extramsec += cls.frametime;
	int ms = extramsec;
	extramsec -= ms;	// fractional part is left for next frame
	if(ms > 250)
		ms = 100;		// time was unreasonable
	cmd.msec = ms;
#endif
}

static usercmd_t&	CL_CreateCmd()
{
	// create new command
	cl.cmds_num++;
	
	usercmd_t& cmd = cl.cmds[cl.cmds_num & CMD_MASK];
	
	cmd.clear();
		
	// get basic movement from keyboard
	CL_BaseMove(cmd);

	CL_MouseMove(cmd);

	CL_FinishMove(cmd);
	
	return cmd;
}


void	IN_CenterView()
{
	cl.view_angles[PITCH] = -cl.frame.playerstate.pmove.delta_angles[PITCH];
}

void	CL_InitInput()
{
	cl_upspeed		= Cvar_Get("cl_upspeed", "200", CVAR_NONE);
	cl_forwardspeed		= Cvar_Get("cl_forwardspeed", "200", CVAR_NONE);
	cl_sidespeed		= Cvar_Get("cl_sidespeed", "200", CVAR_NONE);
	
	cl_yawspeed		= Cvar_Get("cl_yawspeed", "140", CVAR_NONE);
	cl_pitchspeed		= Cvar_Get("cl_pitchspeed", "150", CVAR_NONE);
	
	cl_anglespeedkey	= Cvar_Get("cl_anglespeedkey", "1.5", CVAR_NONE);
	cl_run			= Cvar_Get("cl_run", "0", CVAR_ARCHIVE);
	
	m_sensitivity		= Cvar_Get("m_sensitivity", "3", CVAR_ARCHIVE);
	m_filter 		= Cvar_Get("m_filter", "0", CVAR_NONE);
	m_pitch			= Cvar_Get("m_pitch", "0.022", CVAR_ARCHIVE);
	m_yaw			= Cvar_Get("m_yaw", "0.022", CVAR_ARCHIVE);
	m_forward		= Cvar_Get("m_forward", "1", 0);
	m_side			= Cvar_Get("m_side", "1", 0);
	
	

	Cmd_AddCommand("centerview",	IN_CenterView);

	Cmd_AddCommand("+moveup",	IN_UpDown);
	Cmd_AddCommand("-moveup",	IN_UpUp);
	Cmd_AddCommand("+movedown",	IN_DownDown);
	Cmd_AddCommand("-movedown",	IN_DownUp);
	Cmd_AddCommand("+left",		IN_LeftDown);
	Cmd_AddCommand("-left",		IN_LeftUp);
	Cmd_AddCommand("+right",	IN_RightDown);
	Cmd_AddCommand("-right",	IN_RightUp);
	Cmd_AddCommand("+forward",	IN_ForwardDown);
	Cmd_AddCommand("-forward",	IN_ForwardUp);
	Cmd_AddCommand("+back",		IN_BackDown);
	Cmd_AddCommand("-back",		IN_BackUp);
	Cmd_AddCommand("+lookup",	IN_LookupDown);
	Cmd_AddCommand("-lookup",	IN_LookupUp);
	Cmd_AddCommand("+lookdown",	IN_LookdownDown);
	Cmd_AddCommand("-lookdown",	IN_LookdownUp);
	Cmd_AddCommand("+strafe",	IN_StrafeDown);
	Cmd_AddCommand("-strafe",	IN_StrafeUp);
	Cmd_AddCommand("+moveleft",	IN_MoveleftDown);
	Cmd_AddCommand("-moveleft",	IN_MoveleftUp);
	Cmd_AddCommand("+moveright",	IN_MoverightDown);
	Cmd_AddCommand("-moveright",	IN_MoverightUp);
	Cmd_AddCommand("+speed",	IN_SpeedDown);
	Cmd_AddCommand("-speed",	IN_SpeedUp);
	Cmd_AddCommand("+attack1",	IN_Attack1Down);
	Cmd_AddCommand("-attack1",	IN_Attack1Up);
	Cmd_AddCommand("+attack2",	IN_Attack2Down);
	Cmd_AddCommand("-attack2",	IN_Attack2Up);
	Cmd_AddCommand("+use",		IN_UseDown);
	Cmd_AddCommand("-use",		IN_UseUp);
}

void	CL_MouseEvent(int dx, int dy)
{
	in_mx += dx;
	in_my += dy;
}


void	CL_SendCmd()
{
	int		i;
	usercmd_t	*cmd, *oldcmd;

	// build a command even if not connected

	// save this command off for prediction
	i = cls.netchan.getOutgoingSequence() & CMD_MASK;
	cmd = &cl.cmds[i];
	cl.cmd_time[i] = cls.realtime;	// for netgraph ping calculation

	*cmd = CL_CreateCmd();

	if(cls.state == CA_DISCONNECTED || cls.state == CA_CONNECTING || cls.demo_playback)
		return;

	if(cls.state == CA_CONNECTED)
	{
		// just update reliable	if needed
		if(cls.netchan.message.getCurSize() || Sys_Milliseconds() - cls.netchan.getLastSent() > 1000)
			cls.netchan.transmit(bitmessage_c());
			
		return;
	}

	// send a userinfo update if needed
	if(cvar_userinfo_modified)
	{
		CL_FixUpGender();
		cvar_userinfo_modified = false;
		cls.netchan.message.writeBits(CLC_USERINFO, clc_bitcount);
		cls.netchan.message.writeString(Cvar_Userinfo());
	}

	bitmessage_c msg(MAX_PACKETLEN*8);

	// begin a client move command
	msg.writeBits(CLC_MOVE, clc_bitcount);

	// let the server know what the last frame we
	// got was, so the next message can be delta compressed
	if(!cl.frame.valid || cls.demo_waiting)
		msg.writeLong(-1);	// no compression
	else
		msg.writeLong(cl.frame.serverframe);

	// send this and the previous cmds in the message, so
	// if the last packet was dropped, it can be recovered
	i = (cls.netchan.getOutgoingSequence()-2) & CMD_MASK;
	cmd = &cl.cmds[i];
	msg.writeDeltaUsercmd(&null_usercmd, cmd);
	oldcmd = cmd;

	i = (cls.netchan.getOutgoingSequence()-1) & CMD_MASK;
	cmd = &cl.cmds[i];
	msg.writeDeltaUsercmd(oldcmd, cmd);
	oldcmd = cmd;

	i = (cls.netchan.getOutgoingSequence()) & CMD_MASK;
	cmd = &cl.cmds[i];
	msg.writeDeltaUsercmd(oldcmd, cmd);
	
	// deliver the message
	cls.netchan.transmit(msg);
}


