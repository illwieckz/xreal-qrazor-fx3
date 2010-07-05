package xreal.server.game;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.Hashtable;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import javax.vecmath.Vector3f;
import javax.xml.transform.TransformerConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import xreal.CVars;
import xreal.CollisionBspReader;
import xreal.Engine;
import xreal.common.Config;
import xreal.common.ConfigStrings;
import xreal.common.GameType;
import xreal.common.xml.XMLUtils;
import xreal.server.Server;

import com.bulletphysics.collision.broadphase.BroadphaseInterface;
import com.bulletphysics.collision.broadphase.DbvtBroadphase;
import com.bulletphysics.collision.dispatch.CollisionDispatcher;
import com.bulletphysics.collision.dispatch.CollisionObject;
import com.bulletphysics.collision.dispatch.DefaultCollisionConfiguration;
import com.bulletphysics.collision.shapes.CollisionShape;
import com.bulletphysics.dynamics.DiscreteDynamicsWorld;
import com.bulletphysics.dynamics.DynamicsWorld;
import com.bulletphysics.dynamics.RigidBody;
import com.bulletphysics.dynamics.constraintsolver.ConstraintSolver;
import com.bulletphysics.dynamics.constraintsolver.SequentialImpulseConstraintSolver;




public class Game implements GameListener {
	
	static private int levelTime = 0;
	static private int deltaTime = 0;
	
	static private Set<GameEntity> entities;
	static private Set<Player> players;
	
	// keep the collision shapes, for deletion/cleanup
	static private List<CollisionShape> collisionShapes;
	static private BroadphaseInterface broadphase;
	static private CollisionDispatcher dispatcher;
	static private ConstraintSolver solver;
	static private DefaultCollisionConfiguration collisionConfiguration;
	
	// 
	private static DynamicsWorld dynamicsWorld = null;
	
	// maximum number of objects (and allow user to shoot additional boxes)
	private static final int MAX_PROXIES = 1024;
	
	//
	private static Hashtable<String, Document> documentHashtable;
	
	/**  */
	private static String entitiesString;
	
	private static GameClassFactory classFactory = null;

	private Game() {
		
	}

	@Override
	public boolean consoleCommand() {
		//Engine.print("xreal.server.game.Game.consoleCommand()\n");
		
		String cmd = Engine.getConsoleArgv(0);
		
		if(CVars.g_dedicated.getBoolean())
		{
			if(cmd.equals("say"))
			{
				Server.broadcastClientCommand("print \"server: " + Engine.concatConsoleArgs(1)  + "\n\"");
				return true;
			}
			
			// everything else will also be printed as a say command
			Server.broadcastClientCommand("print \"server: " + Engine.concatConsoleArgs(0) + "\n\"");
			return true;
		}
		
		return false;
	}

	@Override
	public void initGame(int levelTime, int randomSeed, boolean restart) {
		
		Engine.print("xreal.server.game.Game.initGame(levelTime = "+ levelTime + ", randomSeed = " + randomSeed + ", restart = " + restart + ")\n");
		
		//crashTest();
		
		Engine.print("------- Game Initialization -------\n");
		
		entities = new LinkedHashSet<GameEntity>();
		players = new LinkedHashSet<Player>();
		
		//Engine.println("gamename: "Config.GAME_VERSION);
		//Engine.print("gamedate: %s\n", __DATE__);

		//Engine.sendConsoleCommand(Engine.EXEC_APPEND, "echo cool!");
		
		Game.levelTime = levelTime;
		deltaTime = 0;
		
		// make some data visible to connecting client
		Server.setConfigString(ConfigStrings.GAME_VERSION, Config.GAME_VERSION);
		Server.setConfigString(ConfigStrings.LEVEL_START_TIME, Integer.toString(levelTime));
		Server.setConfigString(ConfigStrings.MOTD, CVars.g_motd.getString());
		
		Engine.println("Game Version: " + Server.getConfigString(ConfigStrings.GAME_VERSION));
		Engine.println("Game Type: " + GameType.values()[CVars.g_gametype.getInteger()]);
		
		initPhysics();
		
		documentHashtable = new Hashtable<String, Document>();
		
		// create the inital, mostly empty level document (don't pass the entString)
		Document levelDoc = GameUtil.buildLevelDocument(CVars.g_mapname.getString(), null, null);
		documentHashtable.put("xreal.level", levelDoc);

		// let interested objects know we're building a new level
		// document, and they may add on to it.
		//gameStatusSupport.fireEvent(GameStatusEvent.GAME_BUILD_LEVEL_DOCUMENT, entString, spawnPoint);

		// parse the entString and place its contents in the document
		// if no spawn entities have been placed in the document yet,
		// or if the <include-default-entities/> tag appears
		Element root = levelDoc.getDocumentElement();
		NodeList nl = root.getElementsByTagName("entity");
		NodeList nl2 = root.getElementsByTagName("include-default-entities");
		if ((nl.getLength() == 0) || (nl2.getLength() > 0))
			GameUtil.parseEntString(root, entitiesString);
		
		/*
		try {
			XMLUtils.writeXMLDocument(levelDoc, "level.xml");
		} catch (TransformerConfigurationException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (Throwable e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		*/

		// let interested objects know we're starting a new map, but nothing's
		// been spawned yet - so that can inspect or tweak the level document.
		//gameStatusSupport.fireEvent(GameStatusEvent.GAME_PRESPAWN);

		// ponder whether or not we need to change player classes
		//rethinkPlayerClass();

		// suggest a gc, to clean things up from the last level, before
		// spawning all the new entities
		System.gc();
		
		// set class factory so we can associate xml entities with our java classes in xreal.server.game.spawn
		classFactory =  new DefaultClassFactory();
		
		String classPaths[] = {"xreal.server.game"};
		classFactory.setPackagePath(classPaths);

		// read through the document and actually create the entities
		spawnEntities();
		
		
		// parse the key/value pairs and spawn gentities
		//spawnEntitiesFromString();
		
//		for(int i = 0; i < 30; i++)
//		{
//			GameEntity e1 = new GameEntity();
//			
//			/*
//			Engine.println(e1.toString());
//			try {
//				e1.finalize();
//			} catch (Throwable e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//			}
//			*/
//		}
		
		
		Engine.print("-----------------------------------\n");
	}

	@Override
	public void runAIFrame(int time) {
		//Engine.print("xreal.server.game.Game.runAIFrame(time = " + time + ")\n");

	}

	@Override
	public void runFrame(int time) {
		//Engine.print("xreal.server.game.Game.runFrame(time = " + time + ")\n");
		
		deltaTime = time - levelTime;
		levelTime = time;
		
		runPhysics();
		
		// go through all allocated objects
		int start = Engine.getTimeInMilliseconds();
		
		for(GameEntity ent : entities)
		{
			/*
			if(!ent.inuse)
			{
				continue;
			}
			*/

			/*
			// clear events that are too old
			if(level.time - ent->eventTime > EVENT_VALID_MSEC)
			{
				if(ent->s.event)
				{
					ent->s.event = 0;	// &= EV_EVENT_BITS;
					if(ent->client)
					{
						ent->client->ps.externalEvent = 0;
						// predicted events should never be set to zero
						//ent->client->ps.events[0] = 0;
						//ent->client->ps.events[1] = 0;
					}
				}
				if(ent->freeAfterEvent)
				{
					// tempEntities or dropped items completely go away after their event
					G_FreeEntity(ent);
					continue;
				}
				else if(ent->unlinkAfterEvent)
				{
					// items that will respawn will hide themselves after their pickup event
					ent->unlinkAfterEvent = qfalse;
					trap_UnlinkEntity(ent);
				}
			}

			// temporary entities don't think
			if(ent->freeAfterEvent)
			{
				continue;
			}

			if(!ent->r.linked && ent->neverFree)
			{
				continue;
			}
			*/

			/*
			if(ent->s.eType == ET_PROJECTILE || ent->s.eType == ET_PROJECTILE2)
			{
				G_RunMissile(ent);
				continue;
			}

			if(ent->s.eType == ET_ITEM || ent->physicsObject)
			{
				G_RunItem(ent);
				continue;
			}

			if(ent->s.eType == ET_MOVER)
			{
				G_RunMover(ent);
				continue;
			}

			if(i < MAX_CLIENTS)
			{
				G_RunClient(ent);
				continue;
			}
			*/

			ent.runThink();
		}
		int end = Engine.getTimeInMilliseconds();

		/*
		start = Engine.getTimeInMilliseconds();

		// perform final fixups on the players
		ent = &g_entities[0];
		for(i = 0; i < level.maxclients; i++, ent++)
		{
			if(ent->inuse)
			{
				ClientEndFrame(ent);
			}
		}
		end = Engine.getTimeInMilliseconds();
		*/

		// see if it is time to do a tournament restart
		//checkTournament();

		// see if it is time to end the level
		//checkExitRules();

		// update to team status?
		//checkTeamStatus();

		// cancel vote if timed out
		//checkVote();

		// check team votes
		//checkTeamVote(TEAM_RED);
		//checkTeamVote(TEAM_BLUE);
		
		//CVars.g_gametype.set("99");
		//CVars.g_gametype = null;
		//Engine.print(CVars.g_gametype.toString() + "\n");
		
		//System.gc();
		
		//crashTest();
		
		//Engine.print("xreal.server.game.Game.runFrame(time2 = " + Engine.getTimeInMilliseconds() + ")\n");
	}

	@Override
	public void shutdownGame(boolean restart) {
		Engine.print("xreal.server.game.Game.shutdownGame(restart = " + restart + ")\n");
		
		// kill all threads
		/*
		if (CVars.g_threadEntities.getBoolean()) {

			for (GameEntity ent : entities) {
				ent.stop();
			}
		}
		*/
	}
	
	static public int getLevelTime() {
		return levelTime;
	}
	
	private void initPhysics() {

		Engine.println("Game.initPhysics()");
		
		collisionShapes = new ArrayList<CollisionShape>();
		
		// collision configuration contains default setup for memory, collision
		// setup
		collisionConfiguration = new DefaultCollisionConfiguration();

		// use the default collision dispatcher. For parallel processing you can
		// use a diffent dispatcher (see Extras/BulletMultiThreaded)
		dispatcher = new CollisionDispatcher(collisionConfiguration);

		// the maximum size of the collision world. Make sure objects stay
		// within these boundaries
		
		
		// Don't make the world AABB size too large, it will harm simulation
		// quality and performance
		//Vector3f worldAabbMin = new Vector3f(-10000, -10000, -10000);
		//Vector3f worldAabbMax = new Vector3f(10000, 10000, 10000);
		//overlappingPairCache = new AxisSweep3(worldAabbMin, worldAabbMax);//, MAX_PROXIES);
		
		 //broadphase = new SimpleBroadphase(MAX_PROXIES);
		
		// new JBullet supports DbvtBroadphase
		broadphase = new DbvtBroadphase();

		// the default constraint solver. For parallel processing you can use a
		// different solver (see Extras/BulletMultiThreaded)
		SequentialImpulseConstraintSolver sol = new SequentialImpulseConstraintSolver();
		solver = sol;

		// TODO: needed for SimpleDynamicsWorld
		// sol.setSolverMode(sol.getSolverMode() &
		// ~SolverMode.SOLVER_CACHE_FRIENDLY.getMask());

		dynamicsWorld = new DiscreteDynamicsWorld(dispatcher, broadphase, solver, collisionConfiguration);
		// dynamicsWorld = new SimpleDynamicsWorld(dispatcher,
		// overlappingPairCache, solver, collisionConfiguration);

		dynamicsWorld.setGravity(new Vector3f(CVars.g_gravityX.getValue(), CVars.g_gravityY.getValue(), CVars.g_gravityZ.getValue()));

		// create a few basic rigid bodies
		// CollisionShape groundShape = new BoxShape(new Vector3f(50f, 50f,
		// 50f));
		/*
		CollisionShape groundShape = new StaticPlaneShape(new Vector3f(0, 0, 1), 0);

		collisionShapes.add(groundShape);

		Transform groundTransform = new Transform();
		groundTransform.setIdentity();
		groundTransform.origin.set(0, -56, 0);
		
		{
			float mass = 0f;

			// rigidbody is dynamic if and only if mass is non zero, otherwise static
			boolean isDynamic = (mass != 0f);

			Vector3f localInertia = new Vector3f(0, 0, 0);
			if (isDynamic) {
				groundShape.calculateLocalInertia(mass, localInertia);
			}

			// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			DefaultMotionState myMotionState = new DefaultMotionState(groundTransform);
			RigidBodyConstructionInfo rbInfo = new RigidBodyConstructionInfo(mass, myMotionState, groundShape, localInertia);
			RigidBody body = new RigidBody(rbInfo);
			//body.

			// add the body to the dynamics world
			dynamicsWorld.addRigidBody(body);
		}
		*/
		
		initCollisionWorld();
		
		//System.gc();
	}
	
	private void initCollisionWorld() {
		Engine.println("Game.initCollisionWorld()");
		
		CollisionBspReader bsp = new CollisionBspReader("maps/" + CVars.g_mapname.getString() + ".bsp");
		
		bsp.addWorldBrushesToSimulation(collisionShapes, dynamicsWorld);
		
		entitiesString = bsp.getEntitiesString();
	}
	
	/**
	 * Read the current level DOM document and spawn the map's entities.
	 */
	private void spawnEntities() {
		Object[] ctorParams = new Object[1];
		Class[] newCtorParamTypes = new Class[1];
		newCtorParamTypes[0] = SpawnArgs.class;

		// look for <entity>..</entity> sections
		NodeList nl = getDocument("xreal.level").getElementsByTagName("entity");
		for (int i = 0; i < nl.getLength(); i++) {
			String className = null;

			try {
				Element e = (Element) nl.item(i);
				className = e.getAttribute("class");
				ctorParams[0] = new SpawnArgs(e);

				// leighd 04/10/99 - altered lookup method to reference class
				// factory
				// directly rather than calling Game.lookup class.
				Class entClass = classFactory.lookupClass(".spawn."	+ className.toLowerCase());
				Constructor ctor = entClass.getConstructor(newCtorParamTypes);

				try {
					ctor.newInstance(ctorParams);
				} catch (InvocationTargetException ite) {
					throw ite.getTargetException();
				}

			} catch (ClassNotFoundException cnfe) {
				Engine.println("Couldn't find class to handle: " + className);
			} catch (NoSuchMethodException nsme) {
				Engine.println("Class " + className	+ " doesn't have the right kind of constructor");
			//} catch (InhibitedException ie) {
			} catch (Throwable t) {
				t.printStackTrace();
			}
		}
	}
	
	private void runPhysics() {
		dynamicsWorld.stepSimulation(deltaTime * 0.001f, 10);
		
		//Engine.println("Game.runPhysics(): collision objects = " + dynamicsWorld.getNumCollisionObjects());
		
		dynamicsWorld.setGravity(new Vector3f(CVars.g_gravityX.getValue(), CVars.g_gravityY.getValue(), CVars.g_gravityZ.getValue()));

		// print positions of all objects
		for (int j = dynamicsWorld.getNumCollisionObjects() - 1; j >= 0; j--) {
			
			CollisionObject obj = dynamicsWorld.getCollisionObjectArray().get(j);
			RigidBody body = RigidBody.upcast(obj);
			
			if (body != null && body.getMotionState() != null) {
				
				GameEntity ent = (GameEntity) body.getUserPointer();
				if (ent != null) {
					
					ent.updateEntityStateByPhysics();
					
					/*
					if(body.isActive() && !ent.isAlive()) {
						ent.start();
					}
					*/
				}
			}
		}
	}
	
	private void crashTest() {
		
		//try
		{
			GameEntity ent = null;
		
			ent.updateEntityStateByPhysics();
		
			Vector3f v1 = null;
			Vector3f v2 = new Vector3f(v1);
		}
		/*
		catch(Exception e)
		{
			Engine.println("exception in Game.crashTest(): " + e.getMessage());
		}
		*/
	}
	
	/**
	 * Searches all active entities for the next one that holds
	 * the matching string at fieldofs (use the FOFS() macro) in the structure.
	 * 
	 * Searches beginning at the entity after from, or the beginning if NULL
	 * NULL will be returned if the end of the list is reached.
	 */
	public static GameEntity findEntity(GameEntity from, String className)
	{
		/*
		if(from != null)
			from = g_entities;
		else
			from++;
		*/

		for(GameEntity ent : entities)
		{
			if(ent == null)
				continue;
			
			if(ent == from)
				continue;
			
			//.spawnArgs.getClass();
			
			//if(!ent.isActive())
			//	continue;
			
			if(ent.getClassName().equals(className))
			{
				return ent;
			}
			
		}

		return null;
	}
	
	public static Set<GameEntity> getEntities() {
		return entities;
	}
	
	public static Set<Player> getPlayers() {
		return players;
	}
	
	public static List<CollisionShape> getCollisionShapes() {
		return collisionShapes;
	}
	
	public static BroadphaseInterface getBroadphase() {
		return broadphase;
	}

	public static DynamicsWorld getDynamicsWorld() {
		return dynamicsWorld;
	}
	
	/**
	 * Get one of the DOM documents the Game keeps internally.
	 * 
	 * @return org.w3c.dom.Document
	 * @param documentKey
	 *            java.lang.String
	 */
	public static Document getDocument(String documentKey) {
		return documentHashtable.get(documentKey);
	}
}
