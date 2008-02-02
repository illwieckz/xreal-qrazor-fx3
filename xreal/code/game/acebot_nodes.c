/*
===========================================================================
Copyright (C) 1998 Steve Yeager
Copyright (C) 2008 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
//  acebot_nodes.c -   This file contains all of the 
//                     pathing routines for the ACE bot.


#include "g_local.h"
#include "acebot.h"

#if defined(ACEBOT)

// flags
qboolean        newmap = qtrue;

// Total number of nodes that are items
int             numitemnodes;

// Total number of nodes
int             numnodes;

// For debugging paths
int             show_path_from = -1;
int             show_path_to = -1;

// array for node data
node_t          nodes[MAX_NODES];
short int       path_table[MAX_NODES][MAX_NODES];

///////////////////////////////////////////////////////////////////////
// NODE INFORMATION FUNCTIONS
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Determin cost of moving from one node to another
///////////////////////////////////////////////////////////////////////
int ACEND_FindCost(int from, int to)
{
	int             curnode;
	int             cost = 1;	// Shortest possible is 1

	// If we can not get there then return invalid
	if(path_table[from][to] == INVALID)
		return INVALID;

	// Otherwise check the path and return the cost
	curnode = path_table[from][to];

	// Find a path (linear time, very fast)
	while(curnode != to)
	{
		curnode = path_table[curnode][to];
		if(curnode == INVALID)	// something has corrupted the path abort
			return INVALID;
		cost++;
	}

	return cost;
}

///////////////////////////////////////////////////////////////////////
// Find a close node to the player within dist.
//
// Faster than looking for the closest node, but not very 
// accurate.
///////////////////////////////////////////////////////////////////////
int ACEND_FindCloseReachableNode(gentity_t * self, int range, int type)
{
	vec3_t          v;
	int             i;
	trace_t         tr;
	float           dist;

	range *= range;

	for(i = 0; i < numnodes; i++)
	{
		if(type == NODE_ALL || type == nodes[i].type)	// check node type
		{

			VectorSubtract(nodes[i].origin, self->s.origin, v);	// subtract first

			dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

			if(dist < range)	// square range instead of sqrt
			{
				// make sure it is visible
				trap_Trace(&tr, self->client->ps.origin, self->r.mins, self->r.maxs, nodes[i].origin, self->s.number, MASK_PLAYERSOLID);

				if(tr.fraction == 1.0)
					return i;
			}
		}
	}

	return -1;
}

///////////////////////////////////////////////////////////////////////
// Find the closest node to the player within a certain range
///////////////////////////////////////////////////////////////////////
int ACEND_FindClosestReachableNode(gentity_t * self, int range, int type)
{
	int             i;
	float           closest = 99999;
	float           dist;
	int             node = -1;
	vec3_t          v;
	trace_t         tr;
	float           rng;
	vec3_t          maxs, mins;

	VectorCopy(self->r.mins, mins);
	VectorCopy(self->r.maxs, maxs);

	// For Ladders, do not worry so much about reachability
	if(type == NODE_LADDER)
	{
		VectorCopy(vec3_origin, maxs);
		VectorCopy(vec3_origin, mins);
	}
	else
		mins[2] += 18;			// Stepsize

	rng = (float)(range * range);	// square range for distance comparison (eliminate sqrt)  

	for(i = 0; i < numnodes; i++)
	{
		if(type == NODE_ALL || type == nodes[i].type)	// check node type
		{
			VectorSubtract(nodes[i].origin, self->s.origin, v);	// subtract first

			dist = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];

			if(dist < closest && dist < rng)
			{
				// make sure it is visible
				trap_Trace(&tr, self->client->ps.origin, mins, maxs, nodes[i].origin, self->s.number, MASK_PLAYERSOLID);

				if(tr.fraction == 1.0)
				{
					node = i;
					closest = dist;
				}
			}
		}
	}

	return node;
}

///////////////////////////////////////////////////////////////////////
// BOT NAVIGATION ROUTINES
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Set up the goal
///////////////////////////////////////////////////////////////////////
void ACEND_SetGoal(gentity_t * self, int goal_node)
{
	int             node;

	self->bs.goal_node = goal_node;
	node = ACEND_FindClosestReachableNode(self, NODE_DENSITY * 3, NODE_ALL);

	if(node == -1)
		return;

	if(debug_mode)
		debug_printf("%s new start node selected %d\n", self->client->pers.netname, node);


	self->bs.current_node = node;
	self->bs.next_node = self->bs.current_node;	// make sure we get to the nearest node first
	self->bs.node_timeout = 0;

}

///////////////////////////////////////////////////////////////////////
// Move closer to goal by pointing the bot to the next node
// that is closer to the goal
///////////////////////////////////////////////////////////////////////
qboolean ACEND_FollowPath(gentity_t * self)
{
	vec3_t          v;

#if 1
	// Show the path (uncomment for debugging)
	show_path_from = self->bs.current_node;
	show_path_to = self->bs.goal_node;
	ACEND_DrawPath();
#endif

	// Try again?
	if(self->bs.node_timeout++ > 30)
	{
		if(self->bs.tries++ > 3)
			return qfalse;
		else
			ACEND_SetGoal(self, self->bs.goal_node);
	}

	// Are we there yet?
	VectorSubtract(self->s.origin, nodes[self->bs.next_node].origin, v);

	if(VectorLength(v) < 32)
	{
		// reset timeout
		self->bs.node_timeout = 0;

		if(self->bs.next_node == self->bs.goal_node)
		{
			if(debug_mode)
				debug_printf("%s reached goal!\n", self->client->pers.netname);

			ACEAI_PickLongRangeGoal(self);	// Pick a new goal
		}
		else
		{
			self->bs.current_node = self->bs.next_node;
			self->bs.next_node = path_table[self->bs.current_node][self->bs.goal_node];
		}
	}

	if(self->bs.current_node == -1 || self->bs.next_node == -1)
		return qfalse;

	// Set bot's movement vector
	VectorSubtract(nodes[self->bs.next_node].origin, self->s.origin, self->bs.move_vector);

	return qtrue;
}


///////////////////////////////////////////////////////////////////////
// MAPPING CODE
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Capture when the grappling hook has been fired for mapping purposes.
///////////////////////////////////////////////////////////////////////
/*
void ACEND_GrapFired(gentity_t * self)
{
	int             closest_node;
	gentity_t      *owner;

	if(self->r.ownerNum == ENTITYNUM_NONE)
		return;	

	owner = &g_entities[self->r.ownerNum];

	// should not be here

	// Check to see if the grapple is in pull mode
	if(self->owner->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
	{
		// Look for the closest node of type grapple
		closest_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_GRAPPLE);
		if(closest_node == -1)	// we need to drop a node
		{
			closest_node = ACEND_AddNode(self, NODE_GRAPPLE);

			// Add an edge
			ACEND_UpdateNodeEdge(self->owner->last_node, closest_node);

			self->owner->last_node = closest_node;
		}
		else
			self->owner->last_node = closest_node;	// zero out so other nodes will not be linked
	}
}
*/


///////////////////////////////////////////////////////////////////////
// Check for adding ladder nodes
///////////////////////////////////////////////////////////////////////
/*
qboolean ACEND_CheckForLadder(gentity_t * self)
{
	int             closest_node;

	// If there is a ladder and we are moving up, see if we should add a ladder node
	if(gi.pointcontents(self->s.origin) & CONTENTS_LADDER && self->velocity[2] > 0)
	{
		//debug_printf("contents: %x\n",tr.contents);

		closest_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_LADDER);
		if(closest_node == -1)
		{
			closest_node = ACEND_AddNode(self, NODE_LADDER);

			// Now add link
			ACEND_UpdateNodeEdge(self->last_node, closest_node);

			// Set current to last
			self->last_node = closest_node;
		}
		else
		{
			ACEND_UpdateNodeEdge(self->last_node, closest_node);
			self->last_node = closest_node;	// set visited to last
		}
		return qtrue;
	}
	return qfalse;
}
*/

///////////////////////////////////////////////////////////////////////
// This routine is called to hook in the pathing code and sets
// the current node if valid.
///////////////////////////////////////////////////////////////////////
void ACEND_PathMap(gentity_t * self)
{
	int             closest_node;
	static float    last_update = 0;	// start off low
	vec3_t          v;

	if(level.time < last_update)
		return;

	last_update = level.time + 150;	// slow down updates a bit

	// Special node drawing code for debugging
	if(show_path_to != -1)
		ACEND_DrawPath();

	////////////////////////////////////////////////////////
	// Special check for ladder nodes
	///////////////////////////////////////////////////////
	//if(ACEND_CheckForLadder(self))	// check for ladder nodes
	//	return;

	// Not on ground, and not in the water, so bail
	if(self->s.groundEntityNum == ENTITYNUM_NONE && !self->waterlevel)
		return;

	////////////////////////////////////////////////////////
	// Lava/Slime
	////////////////////////////////////////////////////////
	VectorCopy(self->s.origin, v);
	v[2] -= 18;
	
	if(trap_PointContents(self->r.currentOrigin, -1) & (CONTENTS_LAVA | CONTENTS_SLIME))
		return;					// no nodes in slime

	////////////////////////////////////////////////////////
	// Jumping
	///////////////////////////////////////////////////////
	if(self->bs.is_jumping)
	{
		// See if there is a closeby jump landing node (prevent adding too many)
		closest_node = ACEND_FindClosestReachableNode(self, 64, NODE_JUMP);

		if(closest_node == INVALID)
			closest_node = ACEND_AddNode(self, NODE_JUMP);

		// Now add link
		if(self->bs.last_node != -1)
			ACEND_UpdateNodeEdge(self->bs.last_node, closest_node);

		self->bs.is_jumping = qfalse;
		return;
	}

	////////////////////////////////////////////////////////////
	// Grapple
	// Do not add nodes during grapple, added elsewhere manually
	////////////////////////////////////////////////////////////
	/*
	if(ctf->value && self->client->ctf_grapplestate == CTF_GRAPPLE_STATE_PULL)
		return;
	*/

	// Iterate through all nodes to make sure far enough apart
	closest_node = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);

	////////////////////////////////////////////////////////
	// Special Check for Platforms
	////////////////////////////////////////////////////////
	/* FIXME
	if(self->groundentity && self->groundentity->use == Use_Plat)
	{
		if(closest_node == INVALID)
			return;				// Do not want to do anything here.

		// Here we want to add links
		if(closest_node != self->last_node && self->last_node != INVALID)
			ACEND_UpdateNodeEdge(self->last_node, closest_node);

		self->last_node = closest_node;	// set visited to last
		return;
	}
	*/

	////////////////////////////////////////////////////////
	// Add Nodes as needed
	////////////////////////////////////////////////////////
	if(closest_node == INVALID)
	{
		// Add nodes in the water as needed
		if(self->waterlevel)
			closest_node = ACEND_AddNode(self, NODE_WATER);
		else
			closest_node = ACEND_AddNode(self, NODE_MOVE);

		// Now add link
		if(self->bs.last_node != -1)
			ACEND_UpdateNodeEdge(self->bs.last_node, closest_node);

	}
	else if(closest_node != self->bs.last_node && self->bs.last_node != INVALID)
		ACEND_UpdateNodeEdge(self->bs.last_node, closest_node);

	self->bs.last_node = closest_node;	// set visited to last

}

///////////////////////////////////////////////////////////////////////
// Init node array (set all to INVALID)
///////////////////////////////////////////////////////////////////////
void ACEND_InitNodes(void)
{
	numnodes = 1;
	numitemnodes = 1;
	memset(nodes, 0, sizeof(node_t) * MAX_NODES);
	memset(path_table, INVALID, sizeof(short int) * MAX_NODES * MAX_NODES);

}

///////////////////////////////////////////////////////////////////////
// Show the node for debugging (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_ShowNode(int node)
{
#if 0
	gentity_t        *ent;

	return;						// commented out for now. uncommend to show nodes during debugging,
	// but too many will cause overflows. You have been warned.

	ent = G_Spawn();

	ent->movetype = MOVETYPE_NONE;
	ent->solid = SOLID_NOT;

	if(nodes[node].type == NODE_MOVE)
		ent->s.renderfx = RF_SHELL_BLUE;
	else if(nodes[node].type == NODE_WATER)
		ent->s.renderfx = RF_SHELL_RED;
	else
		ent->s.renderfx = RF_SHELL_GREEN;	// action nodes

	ent->s.modelindex = gi.modelindex("models/items/ammo/grenades/medium/tris.md2");
	ent->owner = ent;
	ent->nextthink = level.time + 200000.0;
	ent->think = G_FreeEdict;
	ent->dmg = 0;

	VectorCopy(nodes[node].origin, ent->s.origin);
	gi.linkentity(ent);
#endif
}

///////////////////////////////////////////////////////////////////////
// Draws the current path (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_DrawPath()
{
#if 1
	int             current_node, goal_node, next_node;

	current_node = show_path_from;
	goal_node = show_path_to;

	next_node = path_table[current_node][goal_node];

	// Now set up and display the path
	while(current_node != goal_node && current_node != -1)
	{
		gentity_t      *ent;
		
		ent = G_Spawn();

		ent->s.eType = ET_BEAM;
		
		VectorCopy(nodes[current_node].origin, ent->s.origin);
		VectorCopy(nodes[next_node].origin, ent->s.origin2);
	
		ent->nextthink = level.time + 10000;
		ent->think = G_FreeEntity;
		
		trap_LinkEntity(ent);
		
		
		current_node = next_node;
		next_node = path_table[current_node][goal_node];
	}
#endif
}

///////////////////////////////////////////////////////////////////////
// Turns on showing of the path, set goal to -1 to 
// shut off. (utility function)
///////////////////////////////////////////////////////////////////////
void ACEND_ShowPath(gentity_t * self, int goal_node)
{
	show_path_from = ACEND_FindClosestReachableNode(self, NODE_DENSITY, NODE_ALL);
	show_path_to = goal_node;
}

///////////////////////////////////////////////////////////////////////
// Add a node of type ?
///////////////////////////////////////////////////////////////////////
int ACEND_AddNode(gentity_t * self, int type)
{
	vec3_t          v1, v2;

	// Block if we exceed maximum
	if(numnodes + 1 > MAX_NODES)
		return qfalse;

	// Set location
	VectorCopy(self->s.origin, nodes[numnodes].origin);

	// Set type
	nodes[numnodes].type = type;

	/////////////////////////////////////////////////////
	// ITEMS
	// Move the z location up just a bit.
	if(type == NODE_ITEM)
	{
		nodes[numnodes].origin[2] += 16;
		numitemnodes++;
	}

	// Teleporters
	if(type == NODE_TELEPORTER)
	{
		// Up 32
		nodes[numnodes].origin[2] += 32;
	}

	if(type == NODE_LADDER)
	{
		nodes[numnodes].type = NODE_LADDER;

		if(debug_mode)
		{
			debug_printf("Node added %d type: Ladder\n", numnodes);
			ACEND_ShowNode(numnodes);
		}

		numnodes++;
		return numnodes - 1;	// return the node added

	}

	// For platforms drop two nodes one at top, one at bottom
	if(type == NODE_PLATFORM)
	{
		VectorCopy(self->r.maxs, v1);
		VectorCopy(self->r.mins, v2);

		// To get the center
		nodes[numnodes].origin[0] = (v1[0] - v2[0]) / 2 + v2[0];
		nodes[numnodes].origin[1] = (v1[1] - v2[1]) / 2 + v2[1];
		nodes[numnodes].origin[2] = self->r.maxs[2];

		if(debug_mode)
			ACEND_ShowNode(numnodes);

		numnodes++;

		nodes[numnodes].origin[0] = nodes[numnodes - 1].origin[0];
		nodes[numnodes].origin[1] = nodes[numnodes - 1].origin[1];
		nodes[numnodes].origin[2] = self->r.mins[2] + 64;

		nodes[numnodes].type = NODE_PLATFORM;

		// Add a link
		ACEND_UpdateNodeEdge(numnodes, numnodes - 1);

		if(debug_mode)
		{
			debug_printf("Node added %d type: Platform\n", numnodes);
			ACEND_ShowNode(numnodes);
		}

		numnodes++;

		return numnodes - 1;
	}

	if(debug_mode)
	{
		if(nodes[numnodes].type == NODE_MOVE)
			debug_printf("Node added %d type: Move\n", numnodes);
		else if(nodes[numnodes].type == NODE_TELEPORTER)
			debug_printf("Node added %d type: Teleporter\n", numnodes);
		else if(nodes[numnodes].type == NODE_ITEM)
			debug_printf("Node added %d type: Item\n", numnodes);
		else if(nodes[numnodes].type == NODE_WATER)
			debug_printf("Node added %d type: Water\n", numnodes);
		else if(nodes[numnodes].type == NODE_GRAPPLE)
			debug_printf("Node added %d type: Grapple\n", numnodes);

		ACEND_ShowNode(numnodes);
	}

	numnodes++;

	return numnodes - 1;		// return the node added
}

///////////////////////////////////////////////////////////////////////
// Add/Update node connections (paths)
///////////////////////////////////////////////////////////////////////
void ACEND_UpdateNodeEdge(int from, int to)
{
	int             i;

	if(from == -1 || to == -1 || from == to)
		return;					// safety

	// Add the link
	path_table[from][to] = to;

	// Now for the self-referencing part, linear time for each link added
	for(i = 0; i < numnodes; i++)
		if(path_table[i][from] != INVALID)
			if(i == to)
				path_table[i][to] = INVALID;	// make sure we terminate
			else
				path_table[i][to] = path_table[i][from];

	if(debug_mode)
		debug_printf("Link %d -> %d\n", from, to);
}

///////////////////////////////////////////////////////////////////////
// Remove a node edge
///////////////////////////////////////////////////////////////////////
void ACEND_RemoveNodeEdge(gentity_t * self, int from, int to)
{
	int             i;

	if(debug_mode)
		debug_printf("%s: Removing Edge %d -> %d\n", self->client->pers.netname, from, to);

	path_table[from][to] = INVALID;	// set to invalid           

	// Make sure this gets updated in our path array
	for(i = 0; i < numnodes; i++)
		if(path_table[from][i] == to)
			path_table[from][i] = INVALID;
}

///////////////////////////////////////////////////////////////////////
// This function will resolve all paths that are incomplete
// usually called before saving to disk
///////////////////////////////////////////////////////////////////////
void ACEND_ResolveAllPaths()
{
	int             i, from, to;
	int             num = 0;

	safe_bprintf(PRINT_HIGH, "Resolving all paths...");

	for(from = 0; from < numnodes; from++)
		for(to = 0; to < numnodes; to++)
		{
			// update unresolved paths
			// Not equal to itself, not equal to -1 and equal to the last link
			if(from != to && path_table[from][to] == to)
			{
				num++;

				// Now for the self-referencing part linear time for each link added
				for(i = 0; i < numnodes; i++)
					if(path_table[i][from] != -1)
						if(i == to)
							path_table[i][to] = -1;	// make sure we terminate
						else
							path_table[i][to] = path_table[i][from];
			}
		}

	safe_bprintf(PRINT_MEDIUM, "done (%d updated)\n", num);
}

///////////////////////////////////////////////////////////////////////
// Save to disk file
//
// Since my compression routines are one thing I did not want to
// release, I took out the compressed format option. Most levels will
// save out to a node file around 50-200k, so compression is not really
// a big deal.
///////////////////////////////////////////////////////////////////////
void ACEND_SaveNodes()
{
	FILE           *pOut;
	char            filename[MAX_QPATH];
	int             i, j;
	int             version = 1;
	char            mapname[MAX_QPATH];

	// Resolve paths
	ACEND_ResolveAllPaths();

	safe_bprintf(PRINT_MEDIUM, "Saving node table...");

	trap_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));

	strcpy(filename, "base\\nav\\");
	strcat(filename, mapname);
	strcat(filename, ".nod");

	if((pOut = fopen(filename, "wb")) == NULL)
		return;					// bail

	fwrite(&version, sizeof(int), 1, pOut);	// write version
	fwrite(&numnodes, sizeof(int), 1, pOut);	// write count
	fwrite(&num_items, sizeof(int), 1, pOut);	// write facts count

	fwrite(nodes, sizeof(node_t), numnodes, pOut);	// write nodes

	for(i = 0; i < numnodes; i++)
		for(j = 0; j < numnodes; j++)
			fwrite(&path_table[i][j], sizeof(short int), 1, pOut);	// write count

	fwrite(item_table, sizeof(item_table_t), num_items, pOut);	// write out the fact table

	fclose(pOut);

	safe_bprintf(PRINT_MEDIUM, "done.\n");
}

///////////////////////////////////////////////////////////////////////
// Read from disk file
///////////////////////////////////////////////////////////////////////
void ACEND_LoadNodes(void)
{
	FILE           *pIn;
	int             i, j;
	char            filename[MAX_QPATH];
	int             version;
	char            mapname[MAX_QPATH];

	trap_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));

	strcpy(filename, "ace\\nav\\");
	strcat(filename, mapname);
	strcat(filename, ".nod");

	if((pIn = fopen(filename, "rb")) == NULL)
	{
		// Create item table
		G_Printf("ACE: No node file found, creating new one...");
		ACEIT_BuildItemNodeTable(qfalse);
		G_Printf("done.\n");
		return;
	}

	// determin version
	fread(&version, sizeof(int), 1, pIn);	// read version

	if(version == 1)
	{
		G_Printf("ACE: Loading node table...");

		fread(&numnodes, sizeof(int), 1, pIn);	// read count
		fread(&num_items, sizeof(int), 1, pIn);	// read facts count

		fread(nodes, sizeof(node_t), numnodes, pIn);

		for(i = 0; i < numnodes; i++)
			for(j = 0; j < numnodes; j++)
				fread(&path_table[i][j], sizeof(short int), 1, pIn);	// write count

		fread(item_table, sizeof(item_table_t), num_items, pIn);
		fclose(pIn);
	}
	else
	{
		// Create item table
		G_Printf("ACE: No node file found, creating new one...");
		ACEIT_BuildItemNodeTable(qfalse);
		G_Printf("done.\n");
		return;					// bail
	}

	G_Printf("done.\n");

	ACEIT_BuildItemNodeTable(qtrue);

}

#endif

