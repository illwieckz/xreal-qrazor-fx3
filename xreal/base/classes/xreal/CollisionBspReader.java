package xreal;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import javax.vecmath.Vector3f;
import javax.vecmath.Vector4f;

import com.bulletphysics.collision.dispatch.CollisionFlags;
import com.bulletphysics.collision.dispatch.CollisionObject;
import com.bulletphysics.collision.shapes.CollisionShape;
import com.bulletphysics.collision.shapes.ConvexHullShape;
import com.bulletphysics.dynamics.DynamicsWorld;
import com.bulletphysics.dynamics.RigidBody;
import com.bulletphysics.dynamics.RigidBodyConstructionInfo;
import com.bulletphysics.linearmath.DefaultMotionState;
import com.bulletphysics.linearmath.GeometryUtil;

import xreal.io.ByteArrayReader;

/**
 * 
 * @author Robert Beckebans
 */
public class CollisionBspReader {

	static final int BSP_IDENT = (('P'<<24)+('S'<<16)+('B'<<8)+'X');  // little-endian "XBSP"
	static final String BSP_IDENT_STRING = "XBSP";
	static final int BSP_VERSION = 48;

	enum LumpType
	{
		ENTITIES,
		SHADERS,
		PLANES,
		NODES,
		LEAFS,
		LEAFSURFACES,
		LEAFBRUSHES,
		MODELS,
		BRUSHES,
		BRUSHSIDES,
		DRAWVERTS,
		DRAWINDEXES,
		FOGS,
		SURFACES,
		LIGHTMAPS,
		LIGHTGRID,
		VISIBILITY
	}

	class Lump
	{
		int             fileofs, filelen;
	}
	
	class Header
	{
		int             ident;
		int             version;

		Lump			lumps[] = new Lump[LumpType.values().length];
	}
	
	class Shader
	{
		String			name;
		int             surfaceFlags;
		int             contentFlags;
	}
	private Shader shaders[];
	
	private Vector4f planes[];
	
	class BrushSide
	{
		Vector4f		plane;
		int             planeNum;
		int             surfaceFlags;
		int             shaderNum;
		//winding_t      *winding;
	}
	private BrushSide brushSides[];
	
	class Brush
	{
		int             shaderNum;	// the shader that determined the contents
		int             contents;
		List<BrushSide>	sides;
	};
	private Brush brushes[];
	
	
	
	
	public CollisionBspReader(String filename)
	{
		Engine.println("CollisionBspReader: loading '" + filename + "'");
		
		byte byteArray[] = Engine.readFile(filename);
		if(byteArray == null) {
			throw new RuntimeException("Could not load '" + filename + "'");
		}
		
		//Engine.println("byteArray length = " + byteArray.length);
		
		//reader = new BufferedReader(new InputStreamReader(new ByteArrayInputStream(byteArray)));
		ByteArrayReader reader = new ByteArrayReader(byteArray);
		
		try {
			
			//int ident = reader.readInt();
			
			byte[] identChars = {'x', 'x', 'x', 'x'};
			reader.read(identChars);
			String ident = new String(identChars);
			
			//Engine.println("'" + ident + "'");
			
			//int indent = (('P'<<24)+('S'<<16)+('B'<<8)+'X');
			
			if(!ident.equals(BSP_IDENT_STRING)) {
				throw new RuntimeException("'" + filename + "' is not a XBSP file: " + ident);
			}
			
			// read header
			Header header = new Header();
			header.version = reader.readInt();
			
			if(header.version != BSP_VERSION) {
				throw new RuntimeException("'" + filename + "' has wrong version number (" + header.version + " should be " + BSP_VERSION +")");
			}
			
			for (int i = 0; i < LumpType.values().length; i++) {
				
				Lump l = new Lump();
				
				l.fileofs = reader.readInt();
				l.filelen = reader.readInt();
				
				//Engine.println(LumpType.values()[i] + ": ofs = " + l.fileofs + ", len = " + l.filelen);
				
				header.lumps[i] = l;
			}
			
			loadShaders(header.lumps[LumpType.SHADERS.ordinal()], byteArray);
			loadPlanes(header.lumps[LumpType.PLANES.ordinal()], byteArray);
			loadBrushSides(header.lumps[LumpType.BRUSHSIDES.ordinal()], byteArray);
			loadBrushes(header.lumps[LumpType.BRUSHES.ordinal()], byteArray);
			
			reader.close();
			
		} catch (IOException e) {
			//e.printStackTrace();
			throw new RuntimeException("Reading Collision BSP failed: " + e.getMessage());
		}
	}
	
	
	void loadShaders(Lump l, byte buf[]) throws IOException
	{
		/*
		typedef struct
		{
			char            shader[64];
			int             surfaceFlags;
			int             contentFlags;
		} dshader_t;
		*/
		
		int dshader_t_size = (64 + 4 + 4);
		if(l.filelen % dshader_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		int count = l.filelen / dshader_t_size;
		
		if(count < 1)
		{
			throw new RuntimeException("Map with no shaders");
		}
		
		ByteArrayReader reader = new ByteArrayReader(buf, l.fileofs, l.filelen);
		
		//Engine.println("lump ofs = " + l.fileofs + ", lump len = " + l.filelen + ", reader.available = " + reader.available());

		Engine.println("CollisionBspReader: loading " + count + " shaders...");
		
		shaders = new Shader[count];
		for(int i = 0; i < shaders.length; i++) {
			
			Shader shader = shaders[i] = new Shader();
			
			byte name[] = new byte[64];
			reader.read(name);
			
			//shader.name = new String(name);
			shader.contentFlags = reader.readInt();
			shader.surfaceFlags = reader.readInt();
			
			//Engine.println("contentFlags = " + shader.contentFlags + ", surfaceFlags = " + shader.surfaceFlags);
		}
		
		//Engine.println("foo");
	}
	
	void loadPlanes(Lump l, byte buf[]) throws IOException
	{
		/*
		typedef struct
		{
			float           normal[3];
			float           dist;
		} dplane_t;
		*/
		
		int dplane_t_size = (3 * 4 + 4);
		if(l.filelen % dplane_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		int count = l.filelen / dplane_t_size;
		
		if(count < 1)
		{
			throw new RuntimeException("Map with no planes");
		}
		
		ByteArrayReader reader = new ByteArrayReader(buf, l.fileofs, l.filelen);
		Engine.println("CollisionBspReader: loading " + count + " planes...");
		
		planes = new Vector4f[count];
		for(int i = 0; i < planes.length; i++) {
			
			Vector4f plane = planes[i] = new Vector4f();
			
			plane.x = reader.readFloat();
			plane.y = reader.readFloat();
			plane.z = reader.readFloat();
			plane.w = -reader.readFloat();
			
			//Engine.println(plane.toString());
		}
	}
	
	void loadBrushSides(Lump l, byte buf[]) throws IOException
	{
		/*
		typedef struct
		{
			int             planeNum;	// positive plane side faces out of the leaf
			int             shaderNum;
		} dbrushside_t;
		*/
		
		int dbrushside_t_size = ( 4 + 4);
		if(l.filelen % dbrushside_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		int count = l.filelen / dbrushside_t_size;
		
		if(count < 1)
		{
			throw new RuntimeException("Map with no brush sides");
		}
		
		ByteArrayReader reader = new ByteArrayReader(buf, l.fileofs, l.filelen);
		Engine.println("CollisionBspReader: loading " + count + " brush sides...");
		
		brushSides = new BrushSide[count];
		for(int i = 0; i < count; i++) {
			
			BrushSide s = brushSides[i] = new BrushSide();
			
			s.planeNum = reader.readInt();
			s.plane = planes[s.planeNum];
			
			s.shaderNum = reader.readInt(); 
			if(s.shaderNum < 0 || s.shaderNum >= shaders.length)
			{
				throw new RuntimeException("bad shaderNum: " + s.shaderNum);
			}
			s.surfaceFlags = shaders[s.shaderNum].surfaceFlags;
		}
	}
	
	void loadBrushes(Lump l, byte buf[]) throws IOException
	{
		/*
		typedef struct
		{
			int             firstSide;
			int             numSides;
			int             shaderNum;	// the shader that determines the contents flags
		} dbrush_t;
		*/
		
		int dbrush_t_size = ( 4 + 4 + 4);
		if(l.filelen % dbrush_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		int count = l.filelen / dbrush_t_size;
		
		if(count < 1)
		{
			throw new RuntimeException("Map with no brushes");
		}
		
		ByteArrayReader reader = new ByteArrayReader(buf, l.fileofs, l.filelen);
		Engine.println("CollisionBspReader: loading " + count + " brushes...");
		
		brushes = new Brush[count];
		for(int i = 0; i < count; i++) {
			
			Brush b = brushes[i] = new Brush();
			
			int firstSide = reader.readInt();
			int numSides = reader.readInt();
			int shaderNum = reader.readInt();
			
			if(shaderNum < 0 || shaderNum >= shaders.length)
			{
				throw new RuntimeException("bad shaderNum: " + shaderNum);
			}
			
			b.sides = new ArrayList<BrushSide>();
			for(int j = 0; j < numSides; j++) {
				b.sides.add(brushSides[firstSide + j]);
			}
		}
	}
	
	
	public void addWorldBrushesToSimulation(List<CollisionShape> collisionShapes, DynamicsWorld dynamicsWorld) {
		//Engine.println("CollisionBspReader.addWorldBrushesToSimulation()");
		
		for(int i = 0; i < brushes.length; i++) {
			
			Brush b = brushes[i];
			
			List<Vector4f> planeEquations = new ArrayList<Vector4f>();
			
			for(BrushSide s : b.sides) {
				planeEquations.add(s.plane);
			}
			
			List<Vector3f> points = new ArrayList<Vector3f>();
			GeometryUtil.getVerticesFromPlaneEquations(planeEquations, points);
			
			//Engine.println("created " + points.size() + " points for brush " + i);
			
			CollisionShape shape = new ConvexHullShape(points);
			
			collisionShapes.add(shape);
			
			// using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
			DefaultMotionState myMotionState = new DefaultMotionState();
			RigidBodyConstructionInfo rbInfo = new RigidBodyConstructionInfo(0, myMotionState, shape);
			RigidBody body = new RigidBody(rbInfo);

			// add the body to the dynamics world
			dynamicsWorld.addRigidBody(body);
			
			/*
			CollisionObject co = new CollisionObject();
			co.setCollisionShape(shape);
			co.setCollisionFlags(CollisionFlags.STATIC_OBJECT);
			dynamicsWorld.addCollisionObject(co);
			*/
		}
	}
}
