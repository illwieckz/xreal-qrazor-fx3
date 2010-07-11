package xreal.server.game;

import javax.vecmath.Quat4f;
import javax.vecmath.Vector3f;

import xreal.Angle3f;
import xreal.CVars;
import xreal.ConsoleColorStrings;
import xreal.Engine;
import xreal.PlayerStateAccess;
import xreal.UserCommand;
import xreal.UserInfo;
import xreal.common.Config;
import xreal.common.ConfigStrings;
import xreal.common.GameType;
import xreal.common.PlayerController;
import xreal.common.PlayerMove;
import xreal.common.PlayerMovementType;
import xreal.common.Team;
import xreal.server.Server;

import com.bulletphysics.collision.broadphase.CollisionFilterGroups;
import com.bulletphysics.collision.dispatch.CollisionFlags;
import com.bulletphysics.collision.dispatch.GhostPairCallback;
import com.bulletphysics.collision.dispatch.PairCachingGhostObject;
import com.bulletphysics.collision.shapes.CapsuleShapeZ;
import com.bulletphysics.collision.shapes.ConvexShape;
import com.bulletphysics.linearmath.Transform;

/**
 * Represents, uses and writes to a native gclient_t
 * 
 * @author Robert Beckebans
 */
public class Player extends GameEntity implements ClientListener, PlayerStateAccess {
	
	UserInfo _userInfo = new UserInfo();
	
	private ClientPersistant	_pers = new ClientPersistant();
	private ClientSession		_sess = new ClientSession();
	
	
	private PairCachingGhostObject		_ghostObject;
	private ConvexShape					_collisionShape;
	private PlayerController			_playerController;
	
	private int					_lastCmdTime;
	
	private boolean				_noClip;
	private boolean				_isBot;
	
	// --------------------------------------------------------------------------------------------
	
	/**
	 * Send a command to the client which will be interpreted by the client game module
	 * 
	 * @param string
	 */
	public synchronized native static void sendServerCommand(int clientNum, String command);
	
	private synchronized static native String 	getUserInfo(int clientNum);
	private synchronized static native void 	setUserInfo(int clientNum, String s);
	
	/**
	 * @return Newest user command.
	 */
	private synchronized static native UserCommand getUserCommand(int clientNum); 
	
	// --------------------------------------------------------------------------------------------
	
	
	Player(int clientNum, boolean firstTime, boolean isBot) throws GameException
	{
		// store the clientNum in the entityState_t::number
		super(clientNum);
		
		_isBot = isBot;
		_sess.sessionTeam = Team.SPECTATOR;
		
		String userinfo = getUserInfo(clientNum);
		if(userinfo.length() == 0)
			userinfo = "\\name\\badinfo";
		
		clientUserInfoChanged(userinfo);
		
		// IP filtering
		// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=500
		// recommanding PB based IP / GUID banning, the builtin system is pretty limited
		// check to see if they are on the banned IP list
		String ip = _userInfo.get("ip");
		/*
		if(G_FilterPacket(ip))
		{
			throw new GameException("You are banned from this server.");
		}
		*/

		// we don't check password for bots and local client
		// NOTE: local client <-> "ip" "localhost"
		//   this means this client is not running in our current process
		if(!isBot && ip != null && !ip.equals("localhost"))
		{
			// check for a password
			String userPassword = _userInfo.get("password");
			String requiredPassword = CVars.g_password.getString();
			
			if(requiredPassword != null && requiredPassword.length() > 0 && !requiredPassword.equals(userPassword))
			{
				throw new GameException("Invalid password");
			}
		}

		_pers.connected = ClientConnectionState.CONNECTING;
		

		// read or initialize the session data
		/*
		if(firstTime || level.newSession)
		{
			G_InitSessionData(client, userinfo);
		}
		G_ReadSessionData(client);
		*/
		
		
		// create player controller ---------------------------------------------------------------
		
		_ghostObject = new PairCachingGhostObject();
		//_ghostObject.setWorldTransform(startTransform);
		
		Game.getBroadphase().getOverlappingPairCache().setInternalGhostPairCallback(new GhostPairCallback());
		
		_collisionShape = new CapsuleShapeZ(Config.PLAYER_WIDTH / 2, Config.PLAYER_HEIGHT / 2);
		//_collisionShape = new BoxShape(new Vector3f(18, 18, 37));
		
		/*
		Vector3f maxs = new Vector3f();
		Vector3f mins = new Vector3f();
		_collisionShape.getAabb(new Transform(), mins, maxs);
		setEntityState_solid(mins, maxs);
		*/
		
		_ghostObject.setCollisionShape(_collisionShape);
		_ghostObject.setCollisionFlags(CollisionFlags.CHARACTER_OBJECT);
		
		_playerController = new PlayerController(_ghostObject, _collisionShape, Game.getDynamicsWorld());
		
		Game.getDynamicsWorld().addCollisionObject(_ghostObject, CollisionFilterGroups.CHARACTER_FILTER, (short)(CollisionFilterGroups.STATIC_FILTER | CollisionFilterGroups.DEFAULT_FILTER));

		Game.getDynamicsWorld().addAction(_playerController);
		
		// ----------------------------------------------------------------------------------------
		
		
		Game.getPlayers().add(this);
	}

	@Override
	public void clientBegin() {
		Engine.print("xreal.server.game.Player.clientBegin(clientNum = " + getEntityState_number() + ")\n");
		
		_pers.connected = ClientConnectionState.CONNECTED;
		_pers.enterTime = Game.getLevelTime();
		_pers.cmd = getUserCommand(getEntityState_number());
		
		//_pers.teamState.state = Team.BEGIN;
		
		// the client has cleared the client side viewangles upon
		// connecting to the server, which is different than the
		// state when the game is saved, so we need to compensate
		// with deltaangles
		Angle3f viewAngles = getPlayerState_viewAngles();
		setPlayerState_deltaPitch(Angle3f.toShort(viewAngles.x));
		setPlayerState_deltaYaw(Angle3f.toShort(viewAngles.y));
		setPlayerState_deltaRoll(Angle3f.toShort(viewAngles.z));
		
		
		spawn();
		
	}
	
	private boolean cheatsOk()
	{
		if(!CVars.sv_cheats.getBoolean())
		{
			sendServerCommand(getEntityState_number(), "print \"Cheats are not enabled on this server.\n\"");
			return false;
		}
		
		/*
		if(_health <= 0)
		{
			sendClientCommand(getEntityState_number(),, "print \"You must be alive to use this command.\n\"");
			return qfalse;
		}
		*/
		
		return true;
	}

	@Override
	public void clientCommand() {
		//Engine.print("xreal.server.game.Player.clientCommand(clientNum = " + getEntityIndex() + ")\n");
		
		String[] args = Engine.getConsoleArgs();
		
		String cmd = args[0];
		
		//Engine.print("xreal.server.game.Player.clientCommand(clientNum = " + getEntityState_number() + ", command '" + cmd + "')\n");
		
		if (cmd.equals("say")) {
			Server.broadcastServerCommand("chat \"" + _pers.netname + ": " + ConsoleColorStrings.GREEN + Engine.concatConsoleArgs(1) + "\n\"");

		} else if (cmd.equals("shootbox")) {

			Vector3f forward = new Vector3f();
			getPlayerState_viewAngles().getVectors(forward, null, null);
			
			GameEntity ent = new TestBox(getPlayerState_origin(), forward);
			//ent.start();
			
		} else if (cmd.equals("shootboxes")) {

			// spawn multiple boxes in front of the player
			
			Vector3f forward = new Vector3f();
			Vector3f right = new Vector3f();
			Vector3f up = new Vector3f();
			
			getPlayerState_viewAngles().getVectors(forward, right, up);
			
			Vector3f origin = getPlayerState_origin();
			Vector3f newOrigin = new Vector3f();
			
			for(int i = -48; i < 48; i += 12)
			{
				newOrigin.scaleAdd(i, right, origin);
				GameEntity ent = new TestBox(newOrigin, forward);
				//ent.start();
			}
			
		} else if (cmd.equals("noclip")) {
			
			if(!cheatsOk()) {
				return;
			}

			String msg;
			if(_noClip)	{
				msg = "noclip OFF\n";
			} else  {
				msg = "noclip ON\n";
			}
			
			_noClip = !_noClip;

			sendServerCommand(getEntityState_number(), "print \"" + msg + "\"");
			
		} else {
			sendServerCommand(getEntityState_number(), "print \"unknown cmd " + cmd + "\n\"");
		}
	}
	
	@Override
	public void clientDisconnect() {
		Engine.print("xreal.server.game.Player.clientDisconnect(clientNum = " + getEntityState_number() + ")\n");
	}

	@Override
	public void clientThink(UserCommand ucmd) {
		//Engine.print("xreal.server.game.Player.clientThink(clientNum = " + getEntityIndex() + ")\n");
		
		//Engine.println(ucmd.toString());
		
		_pers.cmd = ucmd;
		
		// mark the time we got info, so we can display the
		// phone jack if they don't get any for a while
		_lastCmdTime = Game.getLevelTime();
		
		if(!CVars.g_synchronousClients.getBoolean())
		{
			clientThinkReal();
		}
	}
	
	@Override
	public void runThink()
	{
		if(_isBot)
		{
			//TODO ACEAI_Think(ent);
			return;
		}

		clientThinkSynchronous();
	}
	
	private void clientThinkSynchronous()
	{
		if(CVars.g_synchronousClients.getBoolean())
		{
			// don't think if the client is not yet connected (and thus not yet spawned in)
			if(_pers.connected != ClientConnectionState.CONNECTED)
			{
				return;
			}
			
			_pers.cmd.serverTime = Game.getLevelTime();
			
			clientThinkReal();
		}
	}
	
	private void clientThinkReal()
	{
		// don't think if the client is not yet connected (and thus not yet spawned in)
		if(_pers.connected != ClientConnectionState.CONNECTED)
		{
			return;
		}
		
		// mark the time, so the connection sprite can be removed
		UserCommand ucmd = _pers.cmd;

		// sanity check the command time to prevent speedup cheating
		if(ucmd.serverTime > Game.getLevelTime() + 200)
		{
			ucmd.serverTime = Game.getLevelTime() + 200;
			Engine.print("serverTime <<<<<\n");
		}
		
		if(ucmd.serverTime < Game.getLevelTime() - 1000)
		{
			ucmd.serverTime = Game.getLevelTime() - 1000;
			Engine.print("serverTime >>>>>\n");
		}

		int msec = ucmd.serverTime - getPlayerState_commandTime();
		
		// following others may result in bad times, but we still want
		// to check for follow toggles
		if(msec < 1 && _sess.spectatorState != SpectatorState.FOLLOW)
		{
			return;
		}
		
		if(msec > 200)
		{
			msec = 200;
		}

		// check for exiting intermission
		//
		/*
		if(level.intermissiontime)
		{
			ClientIntermissionThink(client);
			return;
		}
		*/
		
		// spectators don't do much
		if(_sess.sessionTeam == Team.SPECTATOR)
		{
			if(_sess.spectatorState == SpectatorState.SCOREBOARD)
			{
				return;
			}
			
			spectatorThink(ucmd);
			return;
		}
		
		// check for inactivity timer, but never drop the local client of a non-dedicated server
		/*
		if(!ClientInactivityTimer(client))
		{
			return;
		}
		*/

		// clear the rewards if time
		/*
		if(level.time > client->rewardTime)
		{
			client->ps.eFlags &=
				~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT | EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP |
				  EF_AWARD_TELEFRAG);
		}
		*/

		if(_noClip)
		{
			setPlayerState_pm_type(PlayerMovementType.NOCLIP);
		}
		/*
		else if(client->ps.stats[STAT_HEALTH] <= 0)
		{
			client->ps.pm_type = PM_DEAD;
		}
		*/
		else
		{
			setPlayerState_pm_type(PlayerMovementType.NORMAL);
		}

		//TODO setPlayerState_gravity(CVars.g_gravityZ);
		
		// set speed
		setPlayerState_speed(CVars.g_speed.getInteger());
		
		
		PlayerMove pm = new PlayerMove(this, ucmd, 0, 0, 0, 0, true, false, 0);
		
		// perform a pmove
		_playerController.movePlayer(pm);
		
		// TODO more movement
	}
	
	private void spectatorThink(UserCommand ucmd) {
		
		//Engine.println("spectatorThink()");
		
		PlayerMove pm = new PlayerMove(this, ucmd, 0, 0, 0, 0, true, false, 0);
		
		//if(_sess.spectatorState != SpectatorState.FOLLOW)
		{
			setPlayerState_pm_type(PlayerMovementType.SPECTATOR);
			setPlayerState_speed(700);	// faster than normal
			
			if(_noClip) {
				setPlayerState_pm_type(PlayerMovementType.NOCLIP);
			}

			// perform a pmove
			_playerController.movePlayer(pm);
			//_playerController.addPlayerMove(pm);

			// save results of pmove
			//VectorCopy(client->ps.origin, ent->s.origin);

			//G_TouchTriggers(ent);
			//trap_UnlinkEntity(ent);
		}

		//client->oldbuttons = client->buttons;
		//client->buttons = ucmd->buttons;

		// attack button cycles through spectators
		/*
		if((client->buttons & BUTTON_ATTACK) && !(client->oldbuttons & BUTTON_ATTACK))
		{
			Cmd_FollowCycle_f(ent, 1);
		}
		*/
	}

	/**
	 * Called from Player() when the player first connects and
	 * directly by the server system when the player updates a userinfo variable.
	 * 
	 * The game can override any of the settings and call Player.setUserinfo
	 * if desired.
	 * 
	 * @param userinfo
	 *            the userinfo string, formatted as:
	 *            "\keyword\value\keyword\value\....\keyword\value"
	 */
	@Override
	public void clientUserInfoChanged(String userinfo) {
		Engine.print("xreal.server.game.Player.clientUserInfoChanged(clientNum = " + getEntityState_number() + ")\n");

		if (userinfo == null)
			return;

		// fill and update the user info hash table
		_userInfo.read(userinfo);

		//Engine.println("Player.userinfo = " + _userInfo.toString());

		// check for local client
		String ip = _userInfo.get("ip");
		if (ip.equals("localhost")) {
			_pers.localClient = true;
		}

		// set name
		String oldname = _pers.netname;
		String name = _userInfo.get("name");

		// TODO _pers.netname = ClientCleanName(name);
		_pers.netname = name;

		if (_sess.sessionTeam == Team.SPECTATOR) {
			if (_sess.spectatorState == SpectatorState.SCOREBOARD) {
				_pers.netname = "scoreboard";
			}
		}

		if (_pers.connected == ClientConnectionState.CONNECTED) {
			if (!_pers.netname.equals(oldname)) {
				Server.broadcastServerCommand("print \"" + oldname + ConsoleColorStrings.WHITE + " renamed to " + _pers.netname + "\n\"");
			}
		}
		
		// set model
		String model = _userInfo.get("model");

		// bots set their team a few frames later
		Team team = _sess.sessionTeam;
		
		GameType gt = GameType.values()[CVars.g_gametype.getInteger()];
		if((gt == GameType.TEAM || gt == GameType.CTF || gt == GameType.ONEFLAG || gt == GameType.OBELISK || gt == GameType.HARVESTER) /* && g_entities[clientNum].r.svFlags & SVF_BOT */)
		{
			/*
			s = Info_ValueForKey(userinfo, "team");
			if(!Q_stricmp(s, "red") || !Q_stricmp(s, "r"))
			{
				team = TEAM_RED;
			}
			else if(!Q_stricmp(s, "blue") || !Q_stricmp(s, "b"))
			{
				team = TEAM_BLUE;
			}
			else
			{
				// pick the team with the least number of players
				team = PickTeam(clientNum);
			}
			*/
		}
		
		// team task (0 = none, 1 = offence, 2 = defence)
		String teamTask = _userInfo.get("teamtask");
		
		
		// team Leader (1 = leader, 0 is normal player)
		boolean teamLeader = _sess.teamLeader;
		

		// colors
		String c1 = _userInfo.get("color1");
		String c2 = _userInfo.get("color2");

		/*
		 * Com_sprintf(userinfo, sizeof(userinfo),
		 * "n\\%s\\t\\%i\\model\\%s\\hmodel\\%s\\g_redteam\\%s\\g_blueteam\\%s\\c1\\%s\\c2\\%s\\hc\\%i\\w\\%i\\l\\%i\\tt\\%d\\tl\\%d"
		 * , client->pers.netname, team, model, "", redTeam, blueTeam, c1, c2,
		 * client->pers.maxHealth, client->sess.wins, client->sess.losses,
		 * teamTask, teamLeader);
		 */
		
		// build new user info CG_NewClientInfo
		
		UserInfo uinfo = new UserInfo();
		uinfo.put("n", _pers.netname);
		uinfo.put("t",  team.toString());
		uinfo.put("model", model);
		uinfo.put("hmodel", "");
		uinfo.put("g_redteam", "");
		uinfo.put("g_redteam", "");
		uinfo.put("c1", c1);
		uinfo.put("c2", c2);
		uinfo.put("hc", _pers.maxHealth);
		uinfo.put("w", _sess.wins);
		uinfo.put("l", _sess.losses);
		uinfo.put("tt", teamTask);
		uinfo.put("tl", teamLeader);
		
		//Engine.println("CS_PLAYERS userinfo = '" + uinfo.toString() + "'");
		
		Server.setConfigString(ConfigStrings.PLAYERS + getEntityState_number(), uinfo.toString());
	}
	
	/**
	 * Called every time a client is placed fresh in the world:
	 * 
	 * After the first ClientBegin, and after each respawn
	 * Initializes all non-persistant parts of playerState
	 */
	private void spawn() {
		
		
		
		
		// find a spawn point
		// do it before setting health back up, so farthest
		// ranging doesn't count this client
		SpawnPoint spawnPoint = null;
		
		//if(_sess.sessionTeam == Team.SPECTATOR)
		{
			spawnPoint = selectSpectatorSpawnPoint();
		}
		
		
		if(spawnPoint != null)
		{
			Vector3f spawnOrigin = spawnPoint.getEntityState_origin();
			
			setOrigin(spawnOrigin);
			setPlayerState_origin(spawnOrigin);
			
			Angle3f spawnAngles = spawnPoint.getEntityState_angles();
			setViewAngles(spawnAngles);
			
			
			Transform startTransform = new Transform();
			startTransform.setIdentity();
			startTransform.origin.set(spawnOrigin);
			
			Quat4f q = new Quat4f();
			spawnAngles.get(q);
			startTransform.setRotation(q);
			
			_ghostObject.setWorldTransform(startTransform);
		}
	}
	
	private void setViewAngles(Angle3f angles) {
		
		/*
			// set the delta angle
			for(i = 0; i < 3; i++)
			{
				int             cmdAngle;
	
				cmdAngle = ANGLE2SHORT(angle[i]);
				ent->client->ps.delta_angles[i] = cmdAngle - ent->client->pers.cmd.angles[i];
			}
			VectorCopy(angle, ent->s.angles);
			VectorCopy(ent->s.angles, ent->client->ps.viewangles);
		 */
		
		setPlayerState_deltaPitch((short)(Angle3f.toShort(angles.x) - _pers.cmd.pitch));
		setPlayerState_deltaYaw((short)(Angle3f.toShort(angles.y) - _pers.cmd.yaw));
		setPlayerState_deltaRoll((short)(Angle3f.toShort(angles.z) - _pers.cmd.roll));

		setPlayerState_viewAngles(angles);
		setEntityState_angles(angles);
		
	}

	/**
	 * This is also used for spectator spawns.
	 */
	private SpawnPoint findIntermissionPoint()
	{
		// find the intermission spot
		GameEntity ent = Game.findEntity(this, "info_player_intermission");
		if(ent == null)
		{
			ent = Game.findEntity(this, "info_player_start");
			if(ent != null)
			{
				return (SpawnPoint) ent;
			}
			else
			{
				// the map creator forgot to put in an intermission point...
				return selectSpawnPoint(new Vector3f());
			}
		}
		else
		{
			
			
				//VectorCopy(ent->s.origin, level.intermission_origin);
				//VectorCopy(ent->s.angles, level.intermission_angle);
	
				// if it has a target, look towards it
				/*
				if(ent->target)
				{
					target = G_PickTarget(ent->target);
					if(target)
					{
						VectorSubtract(target->s.origin, level.intermission_origin, dir);
						VectorToAngles(dir, level.intermission_angle);
					}
				}
				*/
				
				return (SpawnPoint) ent;
			
		}
		
		
	}
	
	private SpawnPoint selectSpectatorSpawnPoint()
	{
		return findIntermissionPoint();
	}
	
	
	/**
	 * Chooses a player start, deathmatch start, etc
	 * 
	 * @param avoidPoint
	 * @return
	 */
	private SpawnPoint selectSpawnPoint(Vector3f avoidPoint)
	{
		return null; //selectRandomFurthestSpawnPoint(avoidPoint);
	}
	
	
	/**
	 * Chooses a player start, deathmatch start, etc
	 * 
	 * @param avoidPoint
	 * @return
	 */
	/*
	private SpawnPoint selectRandomFurthestSpawnPoint(Vector3f avoidPoint)
	{
		SpawnPoint      spot;
		//vec3_t          delta;
		//float           dist;
		//float           list_dist[64];
		//gentity_t      *list_spot[64];
		int             numSpots, rnd, i, j;

		numSpots = 0;
		spot = null;

		//while((spot = ClientGame.G_Find(spot, FOFS(classname), "info_player_deathmatch")) != NULL)
		for(GameEntity ent : Game.getEntities())
		{
			if(ent.getClassName().equals("info_player_start")) {
				
			}
			
			if(SpotWouldTelefrag(spot))
			{
				continue;
			}
			VectorSubtract(spot->s.origin, avoidPoint, delta);
			dist = VectorLength(delta);
			for(i = 0; i < numSpots; i++)
			{
				if(dist > list_dist[i])
				{
					if(numSpots >= 64)
						numSpots = 64 - 1;
					for(j = numSpots; j > i; j--)
					{
						list_dist[j] = list_dist[j - 1];
						list_spot[j] = list_spot[j - 1];
					}
					list_dist[i] = dist;
					list_spot[i] = spot;
					numSpots++;
					if(numSpots > 64)
						numSpots = 64;
					break;
				}
			}
			if(i >= numSpots && numSpots < 64)
			{
				list_dist[numSpots] = dist;
				list_spot[numSpots] = spot;
				numSpots++;
			}
		}
		if(!numSpots)
		{
			spot = G_Find(NULL, FOFS(classname), "info_player_deathmatch");
			if(!spot)
				G_Error("Couldn't find a spawn point");
			VectorCopy(spot->s.origin, origin);
			origin[2] += 9;
			VectorCopy(spot->s.angles, angles);
			return spot;
		}

		// select a random spot from the spawn points furthest away
		rnd = random() * (numSpots / 2);

		VectorCopy(list_spot[rnd]->s.origin, origin);
		origin[2] += 9;
		VectorCopy(list_spot[rnd]->s.angles, angles);

		return list_spot[rnd];
	}
	*/
	
	
	private boolean spotWouldTelefrag(SpawnPoint spot)
	{
		/*
		int             i, num;
		int             touch[MAX_GENTITIES];
		gentity_t      *hit;
		vec3_t          mins, maxs;

		VectorAdd(spot->s.origin, playerMins, mins);
		VectorAdd(spot->s.origin, playerMaxs, maxs);
		num = trap_EntitiesInBox(mins, maxs, touch, MAX_GENTITIES);

		for(i = 0; i < num; i++)
		{
			hit = &g_entities[touch[i]];
			//if ( hit->client && hit->client->ps.stats[STAT_HEALTH] > 0 ) {
			if(hit->client)
			{
				return qtrue;
			}

		}

		return qfalse;
		*/
		
		return false;
	}
	
	// ------------------- playerState_t:: fields in gclient_t::ps --------------------------------
	
	private synchronized static native int getPlayerState_commandTime(int clientNum);

	private synchronized static native void setPlayerState_commandTime(int clientNum, int commandTime);

	private synchronized static native int getPlayerState_pm_type(int clientNum);

	private synchronized static native void setPlayerState_pm_type(int clientNum, int pm_type);

	private synchronized static native int getPlayerState_pm_flags(int clientNum);

	private synchronized static native void setPlayerState_pm_flags(int clientNum, int pm_flags);

	private synchronized static native int getPlayerState_pm_time(int clientNum);

	private synchronized static native void setPlayerState_pm_time(int clientNum, int pm_time);

	private synchronized static native int getPlayerState_bobCycle(int clientNum);

	private synchronized static native void setPlayerState_bobCycle(int clientNum, int bobCycle);

	private synchronized static native Vector3f getPlayerState_origin(int clientNum);

	private synchronized static native void setPlayerState_origin(int clientNum, float x, float y, float z);

	private synchronized static native Vector3f getPlayerState_velocity(int clientNum);

	private synchronized static native void setPlayerState_velocity(int clientNum, float x, float y, float z);

	private synchronized static native int getPlayerState_weaponTime(int clientNum);

	private synchronized static native void setPlayerState_weaponTime(int clientNum, int weaponTime);

	private synchronized static native int getPlayerState_gravity(int clientNum);

	private synchronized static native void setPlayerState_gravity(int clientNum, int gravity);

	private synchronized static native int getPlayerState_speed(int clientNum);

	private synchronized static native void setPlayerState_speed(int clientNum, int speed);
	
	private synchronized static native Vector3f getPlayerState_deltaAngles(int clientNum);
	
	private synchronized static native void setPlayerState_deltaAngles(int clientNum, int pitch, int yaw, int roll);

	private synchronized static native short getPlayerState_deltaPitch(int clientNum);

	private synchronized static native void setPlayerState_deltaPitch(int clientNum, short deltaPitch);

	private synchronized static native short getPlayerState_deltaYaw(int clientNum);

	private synchronized static native void setPlayerState_deltaYaw(int clientNum, short deltaYaw);

	private synchronized static native short getPlayerState_deltaRoll(int clientNum);

	private synchronized static native void setPlayerState_deltaRoll(int clientNum, short deltaRoll);

	private synchronized static native int getPlayerState_groundEntityNum(int clientNum);

	private synchronized static native void setPlayerState_groundEntityNum(int clientNum, int groundEntityNum);

	private synchronized static native int getPlayerState_legsTimer(int clientNum);

	private synchronized static native void setPlayerState_legsTimer(int clientNum, int legsTimer);

	private synchronized static native int getPlayerState_legsAnim(int clientNum);

	private synchronized static native void setPlayerState_legsAnim(int clientNum, int legsAnim);

	private synchronized static native int getPlayerState_torsoTimer(int clientNum);

	private synchronized static native void setPlayerState_torsoTimer(int clientNum, int torsoTimer);

	private synchronized static native int getPlayerState_torsoAnim(int clientNum);

	private synchronized static native void setPlayerState_torsoAnim(int clientNum, int torsoAnim);

	private synchronized static native int getPlayerState_movementDir(int clientNum);

	private synchronized static native void setPlayerState_movementDir(int clientNum, int movementDir);

	private synchronized static native Vector3f getPlayerState_grapplePoint(int clientNum);

	private synchronized static native void setPlayerState_grapplePoint(int clientNum, Vector3f grapplePoint);

	private synchronized static native int getPlayerState_eFlags(int clientNum);

	private synchronized static native void setPlayerState_eFlags(int clientNum, int flags);

	private synchronized static native int getPlayerState_eventSequence(int clientNum);

	private synchronized static native void setPlayerState_eventSequence(int clientNum, int eventSequence);

	private synchronized static native int getPlayerState_externalEvent(int clientNum);

	private synchronized static native void setPlayerState_externalEvent(int clientNum, int externalEvent);

	private synchronized static native int getPlayerState_externalEventParm(int clientNum);

	private synchronized static native void setPlayerState_externalEventParm(int clientNum, int externalEventParm);

	private synchronized static native int getPlayerState_externalEventTime(int clientNum);

	private synchronized static native void setPlayerState_externalEventTime(int clientNum, int externalEventTime);

//	private synchronized static native int getPlayerState_clientNum(int clientNum);

//	private synchronized static native void setPlayerState_clientNum(int clientNum, int clientNum);

	private synchronized static native int getPlayerState_weapon(int clientNum);

	private synchronized static native void setPlayerState_weapon(int clientNum, int weapon);

	private synchronized static native int getPlayerState_weaponState(int clientNum);

	private synchronized static native void setPlayerState_weaponState(int clientNum, int weaponState);

	private synchronized static native Angle3f getPlayerState_viewAngles(int clientNum);
	
	private synchronized static native void setPlayerState_viewAngles(int clientNum, float pitch, float yaw, float roll);

	private synchronized static native int getPlayerState_viewHeight(int clientNum);

	private synchronized static native void setPlayerState_viewHeight(int clientNum, int viewHeight);

	private synchronized static native int getPlayerState_damageEvent(int clientNum);

	private synchronized static native void setPlayerState_damageEvent(int clientNum, int damageEvent);

	private synchronized static native int getPlayerState_damageYaw(int clientNum);

	private synchronized static native void setPlayerState_damageYaw(int clientNum, int damageYaw);

	private synchronized static native int getPlayerState_damagePitch(int clientNum);

	private synchronized static native void setPlayerState_damagePitch(int clientNum, int damagePitch);

	private synchronized static native int getPlayerState_damageCount(int clientNum);

	private synchronized static native void setPlayerState_damageCount(int clientNum, int damageCount);

	private synchronized static native int getPlayerState_generic1(int clientNum);

	private synchronized static native void setPlayerState_generic1(int clientNum, int generic1);

	private synchronized static native int getPlayerState_loopSound(int clientNum);

	private synchronized static native void setPlayerState_loopSound(int clientNum, int loopSound);

	private synchronized static native int getPlayerState_jumppad_ent(int clientNum);

	private synchronized static native void setPlayerState_jumppad_ent(int clientNum, int jumppad_ent);

	private synchronized static native int getPlayerState_ping(int clientNum);

	private synchronized static native void setPlayerState_ping(int clientNum, int ping);
	
	@Override
	public int getPlayerState_bobCycle() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_clientNum() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_commandTime() {
		return getPlayerState_commandTime(getEntityState_number());
	}
	@Override
	public int getPlayerState_damageCount() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_damageEvent() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_damagePitch() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_damageYaw() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public Vector3f getPlayerState_deltaAngles() {
		return getPlayerState_deltaAngles(getEntityState_number());
	}
	
	@Override
	public short getPlayerState_deltaPitch() {
		return getPlayerState_deltaPitch(getEntityState_number());
	}
	
	@Override
	public short getPlayerState_deltaRoll() {
		return getPlayerState_deltaRoll(getEntityState_number());
	}
	
	@Override
	public short getPlayerState_deltaYaw() {
		return getPlayerState_deltaYaw(getEntityState_number());
	}
	
	@Override
	public int getPlayerState_eFlags() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_eventSequence() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_externalEvent() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_externalEventParm() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_externalEventTime() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_generic1() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public Vector3f getPlayerState_grapplePoint() {
		// TODO Auto-generated method stub
		return null;
	}
	@Override
	public int getPlayerState_gravity() {
		return 0; //getPlayerState_gravity(getEntityState_number());
	}
	@Override
	public int getPlayerState_groundEntityNum() {
		return getPlayerState_groundEntityNum(getEntityState_number());
	}
	@Override
	public int getPlayerState_jumppad_ent() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_legsAnim() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_legsTimer() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_loopSound() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_movementDir() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public Vector3f getPlayerState_origin() {
		return getPlayerState_origin(getEntityState_number());
	}
	
	@Override
	public int getPlayerState_ping() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public int getPlayerState_pm_flags() {
		return getPlayerState_pm_flags(getEntityState_number());
	}
	
	@Override
	public int getPlayerState_pm_time() {
		return getPlayerState_pm_time(getEntityState_number());
	}
	
	@Override
	public PlayerMovementType getPlayerState_pm_type() {
		return PlayerMovementType.values()[getPlayerState_pm_type(getEntityState_number())];
	}
	
	@Override
	public int getPlayerState_speed() {
		return getPlayerState_speed(getEntityState_number());
	}
	@Override
	public int getPlayerState_torsoAnim() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_torsoTimer() {
		// TODO Auto-generated method stub
		return 0;
	}
	
	@Override
	public Vector3f getPlayerState_velocity() {
		return getPlayerState_velocity(getEntityState_number());
	}
	
	@Override
	public Angle3f getPlayerState_viewAngles() {
		return getPlayerState_viewAngles(getEntityState_number());
	}
	@Override
	public int getPlayerState_viewHeight() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_weapon() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_weaponState() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public int getPlayerState_weaponTime() {
		// TODO Auto-generated method stub
		return 0;
	}
	@Override
	public void setPlayerState_bobCycle(int bobCycle) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_clientNum(int clientNum) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_commandTime(int time) {
		setPlayerState_commandTime(getEntityState_number(), time);
	}
	
	@Override
	public void setPlayerState_damageCount(int damageCount) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_damageEvent(int damageEvent) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_damagePitch(int damagePitch) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_damageYaw(int damageYaw) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_deltaPitch(short deltaPitch) {
		 setPlayerState_deltaPitch(getEntityState_number(), deltaPitch);
	}
	
	@Override
	public void setPlayerState_deltaRoll(short deltaRoll) {
		setPlayerState_deltaRoll(getEntityState_number(), deltaRoll);
	}
	
	@Override
	public void setPlayerState_deltaYaw(short deltaYaw) {
		setPlayerState_deltaYaw(getEntityState_number(), deltaYaw);	
	}
	
	@Override
	public void setPlayerState_eFlags(int flags) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_eventSequence(int eventSequence) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_externalEvent(int externalEvent) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_externalEventParm(int externalEventParm) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_externalEventTime(int externalEventTime) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_generic1(int generic1) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_grapplePoint(Vector3f grapplePoint) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_gravity(int gravity) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_groundEntityNum(int groundEntityNum) {
		setPlayerState_groundEntityNum(getEntityState_number(), groundEntityNum);
		
	}
	@Override
	public void setPlayerState_jumppad_ent(int jumppad_ent) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_legsAnim(int legsAnim) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_legsTimer(int legsTimer) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_loopSound(int loopSound) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_movementDir(int movementDir) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_origin(Vector3f origin) {
		setPlayerState_origin(getEntityState_number(), origin.x, origin.y, origin.z);	
	}
	
	@Override
	public void setPlayerState_ping(int ping) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_pm_flags(int flags) {
		setPlayerState_pm_flags(getEntityState_number(), flags);
	}

	@Override
	public void setPlayerState_pm_time(int time) {
		setPlayerState_pm_time(getEntityState_number(), time);
	}

	@Override
	public void setPlayerState_pm_type(PlayerMovementType type) {
		setPlayerState_pm_type(getEntityState_number(), type.ordinal());
	}
	
	@Override
	public void setPlayerState_speed(int speed) {
		setPlayerState_speed(getEntityState_number(), speed);	
	}
	
	@Override
	public void setPlayerState_torsoAnim(int torsoAnim) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_torsoTimer(int torsoTimer) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void setPlayerState_velocity(Vector3f velocity) {
		setPlayerState_velocity(getEntityState_number(), velocity.x, velocity.y, velocity.z);	
	}
	
	@Override
	public void setPlayerState_viewAngles(Angle3f viewAngles) {
		setPlayerState_viewAngles(getEntityState_number(), viewAngles.x, viewAngles.y, viewAngles.z);	
	}
	
	@Override
	public void setPlayerState_viewAngles(float pitch, float yaw, float roll) {
		setPlayerState_viewAngles(getEntityState_number(), pitch, yaw, roll);
	}
	
	@Override
	public void setPlayerState_viewHeight(int viewHeight) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_weapon(int weapon) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_weaponState(int weaponState) {
		// TODO Auto-generated method stub
		
	}
	@Override
	public void setPlayerState_weaponTime(int weaponTime) {
		// TODO Auto-generated method stub
		
	}
	
	@Override
	public void addPlayerState_pm_flags(int pm_flags) {
		setPlayerState_pm_flags(getPlayerState_pm_flags() | pm_flags);
	}
	
	@Override
	public void delPlayerState_pm_flags(int pm_flags) {
		setPlayerState_pm_flags(getPlayerState_pm_flags() & ~pm_flags);
	}
	
	@Override
	public boolean hasPlayerState_pm_flags(int pm_flags) {
		return (getPlayerState_pm_flags() & pm_flags) != 0;
	}
	
	// --------------------------------------------------------------------------------------------
}
