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
// xreal --------------------------------------------------------------------
#include "g_local.h"
#include "g_player.h"
#include "g_item.h"
#include "g_target.h"



void	G_BeginIntermission(const g_target_changelevel_c *target)
{
	int		i, n;
	g_entity_c	*spot;
	g_player_c	*player;

	if(level.intermission_time)
		return;		// already activated

	game.autosaved = false;

	//
	// respawn any dead clients
	//
	for(i=0; i<maxclients->getInteger(); i++)
	{
		player = (g_player_c*)g_entities[1+i];
		
		if(!player->_r.inuse)
			continue;
			
		if(player->_health <= 0)
			player->respawn();
	}

	level.intermission_time = level.time;
	level.changemap = target->getMapName();

	if(level.changemap.find("*"))
	{
		if (coop->getInteger())
		{
			for(i=0; i<maxclients->getInteger(); i++)
			{
				player = (g_player_c*)g_entities[1+i];
				
				if(!player->_r.inuse)
					continue;
					
				// strip players of all keys between units
				for(n=0; n < (int)g_items.size()/*MAX_ITEMS*/; n++)
				{
					if(g_items[n]->getFlags() & IT_KEY)
						player->_pers.inventory[n] = 0;
				}
			}
		}
	}
	else
	{
		if(!deathmatch->getInteger())
		{
			level.intermission_exit = true;		// go immediately to the next level
			return;
		}
	}

	level.intermission_exit = false;


	//
	// find an intermission spot
	//
	spot = G_FindByClassName(NULL, "info_player_intermission");
	if(!spot)
	{	
		// the map creator forgot to put in an intermission point...
		spot = G_FindByClassName(NULL, "info_player_start");
		
		if(!spot)
			spot = G_FindByClassName(NULL, "info_player_deathmatch");
	}
	else
	{	// chose one of four spots
		i = rand() & 3;
		while(i--)
		{
			spot = G_FindByClassName(spot, "info_player_intermission");
			
			if(!spot)	// wrap around the list
				spot = G_FindByClassName(spot, "info_player_intermission");
		}
	}

	level.intermission_origin = spot->_s.origin;
	level.intermission_angles = spot->_angles;


	//
	// move all clients to the intermission point
	//
	for(i=0; i<maxclients->getInteger(); i++)
	{
		player = (g_player_c*)g_entities[1+i];
		
		if(!player->_r.inuse)
			continue;
			
		player->moveToIntermission();
	}
}


void	DeathmatchScoreboardMessage()
{
	char		entry[1024];
	char		string[1400];
	int		stringlength;
	int		i, j, k;
	int		sorted[MAX_CLIENTS];
	int		sortedscores[MAX_CLIENTS];
	int		score, total;

	int		x, y;
	
	g_player_c	*player;


	//
	// sort the clients by score
	//
	total = 0;
	for(i=0; i<game.maxclients; i++)
	{
		player = (g_player_c*)g_entities[1+i];
		
		if(!player->_r.inuse || player->_resp.spectator)
			continue;
			
		score = player->_resp.score;
		
		for(j=0; j<total; j++)
		{
			if(score > sortedscores[j])
				break;
		}
		for(k=total; k>j ; k--)
		{
			sorted[k] = sorted[k-1];
			sortedscores[k] = sortedscores[k-1];
		}
		sorted[j] = i;
		sortedscores[j] = score;
		total++;
	}


	//
	// print level name and exit rules
	//
	string[0] = 0;

	stringlength = strlen(string);


	//
	// add the clients in sorted order
	//
	if(total > 12)
		total = 12;

	for(i=0; i<total; i++)
	{
		player = (g_player_c*)g_entities[1 + sorted[i]];

		x = (i>=6) ? 160 : 0;
		y = 32 + 32 * (i%6);

		// send the layout						
		Com_sprintf(entry, sizeof(entry), "client %i %i %i %i %i %i ", x, y, sorted[i], player->_resp.score, player->_r.ping, (level.framenum - player->_resp.enterframe)/600);
		j = strlen(entry);
	
		if(stringlength + j > 1024)
			break;
			
		strcpy(string + stringlength, entry);
		stringlength += j;
	}

	gi.SV_WriteByte(SVC_LAYOUT);
	gi.SV_WriteString(string);
}


/*
==================
DeathmatchScoreboard

Draw instead of help message.
Note that it isn't that hard to overflow the 1400 byte message limit!
==================
*/
void	DeathmatchScoreboard(g_entity_c *ent)
{
	DeathmatchScoreboardMessage();
	gi.SV_Unicast(ent, true);
}




