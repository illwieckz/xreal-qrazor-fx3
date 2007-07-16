/*
===========================================================================
Copyright (C) 2006 Richard "r1ch" Stanway

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

//r1admin(TM), this time actually in the right place !!!
#include "g_local.h"

g_account_t     g_accounts = { 0 };
g_ban_t         g_bans = { 0 };

int Admin_BitsFromMask(unsigned int mask)
{
	int             i;
	int             bits;

	bits = 0;

	for(i = 0; i < 32; i++)
	{
		if(mask & (1 << i))
			bits++;
	}

	return bits;
}

unsigned int Admin_MaskFromBits(int bits)
{
	int             i;
	unsigned int    mask;

	mask = 0xFFFFFFFF;

	for(i = 0; i < 32; i++)
	{
		if(i >= bits)
			mask &= ~(1 << i);
	}

	return mask;
}

unsigned int Admin_IPStringToInt(const char *s)
{
	int             ip[4];
	unsigned int    ret;

	if(sscanf(s, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]) != 4)
		return (unsigned int)-1;

	ret = (ip[0]) + (ip[1] << 8) + (ip[2] << 16) + (ip[3] << 24);

	return ret;
}

const char     *Admin_IPIntToString(unsigned int ip)
{
	static char     ipString[32];
	byte            ret[4];

	*(unsigned int *)&ret = ip;

	Com_sprintf(ipString, sizeof(ipString), "%d.%d.%d.%d", ret[0], ret[1], ret[2], ret[3]);
	return ipString;
}

void Admin_LoadAccounts(void)
{
	int             len;
	g_account_t     next, *accounts;
	fileHandle_t    fileHandle;

	if(!g_accountsFile.string[0])
		return;

	accounts = &g_accounts;

	while(accounts->next)
		accounts = accounts->next;

	len = trap_FS_FOpenFile(g_accountsFile.string, &fileHandle, FS_READ);
	if(!fileHandle)
	{
		G_Printf(va(S_COLOR_RED "accounts file not found: %s\n", g_accountsFile.string));
		return;
	}

	if(len % sizeof(g_account_t))
	{
		G_Printf(va(S_COLOR_RED "corrupted accounts file: %s\n", g_accountsFile.string));
		trap_FS_FCloseFile(fileHandle);
		return;
	}

	while(len)
	{
		trap_FS_Read(&next, sizeof(next), fileHandle);
		len -= sizeof(next);

		accounts->next = G_Alloc(sizeof(next));
		accounts = accounts->next;
		*accounts = next;
		accounts->next = NULL;
	}

	trap_FS_FCloseFile(fileHandle);
}

void Admin_WriteAccounts(void)
{
	g_account_t    *accounts;
	fileHandle_t    fileHandle;

	if(!g_accountsFile.string[0])
		return;

	accounts = &g_accounts;

	trap_FS_FOpenFile(g_accountsFile.string, &fileHandle, FS_WRITE);
	if(!fileHandle)
	{
		G_Printf(va(S_COLOR_RED "couldn't write accounts file: %s\n", g_accountsFile.string));
		return;
	}

	while(accounts->next)
	{
		accounts = accounts->next;
		trap_FS_Write(accounts, sizeof(*accounts), fileHandle);
	}

	trap_FS_FCloseFile(fileHandle);
}

void Admin_LoadBans(void)
{
	int             len;
	g_ban_t         next, *bans;
	fileHandle_t    fileHandle;

	if(!g_bansFile.string[0])
		return;

	bans = &g_bans;

	while(bans->next)
		bans = bans->next;

	len = trap_FS_FOpenFile(g_bansFile.string, &fileHandle, FS_READ);
	if(!fileHandle)
	{
		G_Printf(va(S_COLOR_RED "bans file not found: %s\n", g_bansFile.string));
		return;
	}

	if(len % sizeof(g_ban_t))
	{
		G_Printf(va(S_COLOR_RED "corrupted bans file: %s\n", g_bansFile.string));
		trap_FS_FCloseFile(fileHandle);
		return;
	}

	while(len)
	{
		trap_FS_Read(&next, sizeof(next), fileHandle);
		len -= sizeof(next);

		bans->next = G_Alloc(sizeof(next));
		bans = bans->next;
		*bans = next;
		bans->next = NULL;
	}

	trap_FS_FCloseFile(fileHandle);
}

void Admin_WriteBans(void)
{
	g_ban_t        *bans;
	fileHandle_t    fileHandle;

	if(!g_bansFile.string[0])
		return;

	bans = &g_bans;

	trap_FS_FOpenFile(g_bansFile.string, &fileHandle, FS_WRITE);
	if(!fileHandle)
	{
		G_Printf(va(S_COLOR_RED "couldn't write bans file: %s\n", g_bansFile.string));
		return;
	}

	while(bans->next)
	{
		bans = bans->next;
		trap_FS_Write(bans, sizeof(*bans), fileHandle);
	}

	trap_FS_FCloseFile(fileHandle);
}

void Admin_PrintBans(void)
{
	const char     *unit;
	int             mask, timeleft;
	g_ban_t        *s;

	s = &g_bans;

	Admin_ExpireBans();

	G_Printf("+------------------+---------+---------------------------------+\n");
	G_Printf("| IP Address / Mask| Expires |    Reason for ban               |\n");
	G_Printf("+------------------+---------+---------------------------------+\n");

	while(s->next)
	{
		s = s->next;
		mask = Admin_BitsFromMask(s->mask);

		if(s->expiretime)
		{
			timeleft = s->expiretime - trap_RealTime(NULL);
			if(timeleft >= 60 * 60 * 24 * 365.25)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24 * 365.25));
				unit = "years";
			}
			else if(timeleft >= 60 * 60 * 24 * 7)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24 * 7));
				unit = "weeks";
			}
			else if(timeleft >= 60 * 60 * 24)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24));
				unit = "days ";
			}
			else if(timeleft >= 60 * 60)
			{
				timeleft = (int)(timeleft / (60 * 60));
				unit = "hours";
			}
			else if(timeleft >= 60)
			{
				timeleft = (int)(timeleft / (60));
				unit = "mins ";
			}
			else
			{
				unit = "secs ";
			}

			timeleft++;

			G_Printf("|%-15.15s/%2d|%2d %s |%-33.33s|\n", Admin_IPIntToString(s->ip), mask, timeleft, unit, s->reason);
		}
		else
		{
			G_Printf("|%-15.15s/%2d|  never  |%-33.33s|\n", Admin_IPIntToString(s->ip), mask, s->reason);
		}
	}

	G_Printf("+------------------+---------+---------------------------------+\n");
}

void Admin_PrintBansToClient(gentity_t * ent)
{
	const char     *unit;
	int             mask, timeleft;
	g_ban_t        *s;

	s = &g_bans;

	Admin_ExpireBans();

	trap_SendServerCommand(ent - g_entities,
						   "print \"+------------------+---------+---------------------------------+\n"
						   "| IP Address / Mask| Expires |    Reason for ban               |\n"
						   "+------------------+---------+---------------------------------+\n\"");

	while(s->next)
	{
		s = s->next;
		mask = Admin_BitsFromMask(s->mask);

		if(s->expiretime)
		{
			timeleft = s->expiretime - trap_RealTime(NULL);
			if(timeleft >= 60 * 60 * 24 * 365.25)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24 * 365.25));
				unit = "years";
			}
			else if(timeleft >= 60 * 60 * 24 * 7)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24 * 7));
				unit = "weeks";
			}
			else if(timeleft >= 60 * 60 * 24)
			{
				timeleft = (int)(timeleft / (60 * 60 * 24));
				unit = "days ";
			}
			else if(timeleft >= 60 * 60)
			{
				timeleft = (int)(timeleft / (60 * 60));
				unit = "hours";
			}
			else if(timeleft >= 60)
			{
				timeleft = (int)(timeleft / (60));
				unit = "mins ";
			}
			else
			{
				unit = "secs ";
			}

			timeleft++;

			trap_SendServerCommand(ent - g_entities,
								   va("print \"|%-15.15s/%2d|%2d %s |%-33.33s|\n\"", Admin_IPIntToString(s->ip), mask, timeleft,
									  unit, s->reason));
		}
		else
		{
			trap_SendServerCommand(ent - g_entities,
								   va("print \"|%-15.15s/%2d|  never  |%-33.33s|\n\"", Admin_IPIntToString(s->ip), mask,
									  s->reason));
		}
	}

	trap_SendServerCommand(ent - g_entities, "print \"+------------------+---------+---------------------------------+\n\"");
}

g_account_t    *Admin_GetAccount(const char *username, const char *password)
{
	g_account_t    *u;

	u = &g_accounts;

	while(u->next)
	{
		u = u->next;
		if(!strcmp(u->username, username) && !strcmp(u->password, password))
			return u;
	}

	return NULL;
}

qboolean Admin_AddAccount(const char *username, const char *password, unsigned int permissions)
{
	g_account_t    *u;

	u = &g_accounts;

	while(u->next)
	{
		u = u->next;
		if(!Q_stricmp(username, u->username))
			return qfalse;
	}

	u->next = G_Alloc(sizeof(*u));
	u = u->next;

	u->next = NULL;
	Q_strncpyz(u->username, username, sizeof(u->username));
	Q_strncpyz(u->password, password, sizeof(u->password));
	u->permissions = permissions;
	Admin_WriteAccounts();
	return qtrue;
}

qboolean Admin_RemoveAccount(const char *username)
{
	g_account_t    *u, *last;

	u = last = &g_accounts;

	while(u->next)
	{
		u = u->next;
		if(!Q_stricmp(username, u->username))
		{
			int             i;
			gclient_t      *cl;

			for(i = 0; i < g_maxclients.integer; i++)
			{
				cl = level.clients + i;

				if(cl->pers.connected != CON_CONNECTED)
					continue;

				if(cl->pers.account == u)
					cl->pers.account = NULL;
			}

			last->next = u->next;
			free(u);
			Admin_WriteAccounts();
			return qtrue;
		}
		last = u;
	}

	return qfalse;
}

qboolean Admin_HasPermission(gentity_t * ent, int permission)
{
	if(ent->client->pers.account && (ent->client->pers.account->permissions & (1 << permission)))
		return qtrue;

	trap_SendServerCommand(ent - g_entities, "print \"You do not have permission to use this command.\n\"");
	return qfalse;
}

qboolean Admin_RemoveBan(unsigned int ip, unsigned int mask)
{
	g_ban_t        *u, *last;

	u = last = &g_bans;

	while(u->next)
	{
		u = u->next;
		if(u->ip == ip && u->mask == mask)
		{
			last->next = u->next;
			free(u);
			Admin_WriteBans();
			return qtrue;
		}
		last = u;
	}

	return qfalse;
}

void Admin_Ban(unsigned int ip, unsigned int duration, unsigned int mask, const char *reason)
{
	g_ban_t        *s;

	if(ip == (unsigned int)-1)
		return;

	s = &g_bans;

	while(s->next)
		s = s->next;

	s->next = G_Alloc(sizeof(*s));
	s = s->next;

	s->next = NULL;
	s->ip = ip;
	s->mask = mask;
	if(duration == -1)
		s->expiretime = 0;
	else
		s->expiretime = trap_RealTime(NULL) + duration;
	Q_strncpyz(s->reason, reason, sizeof(s->reason));

	Admin_WriteBans();
}

void Admin_BanClient(gclient_t * cl, unsigned int duration, unsigned int mask, const char *reason)
{
	Admin_Ban(Admin_IPStringToInt(cl->pers.ip), duration, mask, reason);
}

void Admin_ExpireBans(void)
{
	g_ban_t        *s, *last;
	unsigned int    now;
	qboolean        doneWork;

	s = last = &g_bans;

	now = trap_RealTime(NULL);
	doneWork = qfalse;

	while(s->next)
	{
		s = s->next;

		if(s->expiretime && s->expiretime < now)
		{
			doneWork = qtrue;
			Com_Printf("Expiring IP ban %s.\n", Admin_IPIntToString(s->ip));
			last->next = s->next;
			free(s);
			s = last;
			continue;
		}

		last = s;
	}

	if(doneWork)
		Admin_WriteBans();
}

g_ban_t        *Admin_BanMatch(const char *ipString)
{
	g_ban_t        *s;
	unsigned int    ip;

	ip = Admin_IPStringToInt(ipString);

	if(ip == (unsigned int)-1)
		return NULL;

	Admin_ExpireBans();

	s = &g_bans;

	while(s->next)
	{
		s = s->next;

		if((ip & s->mask) == (s->ip & s->mask))
			return s;
	}

	return NULL;
}
