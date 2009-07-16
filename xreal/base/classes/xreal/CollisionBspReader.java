package xreal;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.List;

import javax.vecmath.Vector3f;
import javax.vecmath.Vector4f;

import com.bulletphysics.collision.dispatch.CollisionFlags;
import com.bulletphysics.collision.dispatch.CollisionObject;
import com.bulletphysics.collision.shapes.BvhTriangleMeshShape;
import com.bulletphysics.collision.shapes.CollisionShape;
import com.bulletphysics.collision.shapes.ConvexHullShape;
import com.bulletphysics.collision.shapes.TriangleIndexVertexArray;
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
		
		boolean			checked;
	}
	private Brush brushes[];
	
	class Leaf
	{
		int             cluster;
		int             area;

		int             firstLeafBrush;
		int             numLeafBrushes;

		int             firstLeafSurface;
		int             numLeafSurfaces;
	} 
	private Leaf leafs[];
	private int leafBrushes[];
	private int leafSurfaces[];
	
	
	enum SurfaceType
	{
		BAD,
		PLANAR,
		PATCH,
		TRIANGLE_SOUP,
		FLARE,
		FOLIAGE
	}

	class Surface
	{
		SurfaceType     type;

		int             checkcount;	// to avoid repeated testings
		int             surfaceFlags;
		int             contents;
		
		//int				numVerts;
		//int				numIndices;
		
		List<Vector3f>	vertices;
		List<Integer>	indices;
	}
	private Surface surfaces[];
	
	
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
			loadLeafs(header.lumps[LumpType.LEAFS.ordinal()], byteArray);
			loadLeafBrushes(header.lumps[LumpType.LEAFBRUSHES.ordinal()], byteArray);
			loadLeafSurfaces(header.lumps[LumpType.LEAFSURFACES.ordinal()], byteArray);
			loadPlanes(header.lumps[LumpType.PLANES.ordinal()], byteArray);
			loadBrushSides(header.lumps[LumpType.BRUSHSIDES.ordinal()], byteArray);
			loadBrushes(header.lumps[LumpType.BRUSHES.ordinal()], byteArray);
			loadSurfaces(header.lumps[LumpType.SURFACES.ordinal()], header.lumps[LumpType.DRAWVERTS.ordinal()], header.lumps[LumpType.DRAWINDEXES.ordinal()], byteArray);
			
			//CMod_LoadSurfaces(&header.lumps[LUMP_SURFACES], &header.lumps[LUMP_DRAWVERTS], &header.lumps[LUMP_DRAWINDEXES]);
			
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
			shader.surfaceFlags = reader.readInt();
			shader.contentFlags = reader.readInt();
			
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
			b.contents = shaders[shaderNum].contentFlags;
			
			b.sides = new ArrayList<BrushSide>();
			for(int j = 0; j < numSides; j++) {
				
				BrushSide s = brushSides[firstSide + j];
				b.sides.add(s);
				
				//b.contents |= shaders[s.shaderNum].contentFlags;
			}
		}
	}
	
	void loadLeafs(Lump l, byte buf[]) throws IOException
	{
		/*
		typedef struct
		{
			int             cluster;	// -1 = opaque cluster (do I still store these?)
			int             area;
		
			int             mins[3];	// for frustum culling
			int             maxs[3];
		
			int             firstLeafSurface;
			int             numLeafSurfaces;
		
			int             firstLeafBrush;
			int             numLeafBrushes;
		} dleaf_t;
		*/
		
		int dleaf_t_size = (4 + 4 + 3 * 4 + 3 * 4 + 4 + 4 + 4 + 4);
		if(l.filelen % dleaf_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		int count = l.filelen / dleaf_t_size;
		
		if(count < 1)
		{
			throw new RuntimeException("Map with no leafs");
		}
		
		ByteArrayReader reader = new ByteArrayReader(buf, l.fileofs, l.filelen);
		Engine.println("CollisionBspReader: loading " + count + " leafs...");
		
		leafs = new Leaf[count];
		for(int i = 0; i < count; i++) {
			
			Leaf leaf = leafs[i] = new Leaf();
			
			leaf.cluster = reader.readInt();
			leaf.area = reader.readInt();
			
			for(int j = 0; j < 6; j++) {
				reader.readInt();
			}
			
			leaf.firstLeafSurface = reader.readInt();
			leaf.numLeafSurfaces = reader.readInt();
			
			leaf.firstLeafBrush = reader.readInt();
			leaf.numLeafBrushes = reader.readInt();
		}
	}
	
	void loadLeafBrushes(Lump l, byte buf[]) throws IOException
	{
		int dleafbrush_t_size = (4);
		if(l.filelen % dleafbrush_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		int count = l.filelen / dleafbrush_t_size;
		
		if(count < 1)
		{
			throw new RuntimeException("Map with no leaf brushes");
		}
		
		ByteArrayReader reader = new ByteArrayReader(buf, l.fileofs, l.filelen);
		Engine.println("CollisionBspReader: loading " + count + " leaf brushes...");
		
		leafBrushes = new int[count];
		for(int i = 0; i < count; i++) {
			leafBrushes[i] = reader.readInt();
		}
	}
	
	void loadLeafSurfaces(Lump l, byte buf[]) throws IOException
	{
		int dleafsurface_t_size = (4);
		if(l.filelen % dleafsurface_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		int count = l.filelen / dleafsurface_t_size;
		/*
		if(count < 1)
		{
			throw new RuntimeException("Map with no leaf surfaces");
		}
		*/
		
		ByteArrayReader reader = new ByteArrayReader(buf, l.fileofs, l.filelen);
		Engine.println("CollisionBspReader: loading " + count + " leaf surfaces...");
		
		leafSurfaces = new int[count];
		for(int i = 0; i < count; i++) {
			leafSurfaces[i] = reader.readInt();
		}
	}
	
	void loadSurfaces(Lump surfsLump, Lump vertsLump, Lump indexesLump, byte buf[]) throws IOException
	{
		/*
		typedef struct
		{
			int             shaderNum;
			int             fogNum;
			int             surfaceType;
		
			int             firstVert;
			int             numVerts;
		
			int             firstIndex;
			int             numIndexes;
		
			int             lightmapNum;
			int             lightmapX, lightmapY;
			int             lightmapWidth, lightmapHeight;
		
			float           lightmapOrigin[3];
			float           lightmapVecs[3][3];	// for patches, [0] and [1] are lodbounds
		
			int             patchWidth;
			int             patchHeight;
		} dsurface_t;
		
		typedef struct
		{
			float           xyz[3];
			float           st[2];
			float           lightmap[2];
			float           normal[3];
			float			paintColor[4];
			float           lightColor[4];
			float			lightDirection[3];
		} drawVert_t;
		*/
		
		final int MAX_PATCH_SIZE = 64;
		final int MAX_PATCH_VERTS = (MAX_PATCH_SIZE * MAX_PATCH_SIZE);
		
		//drawVert_t     *dv, *dv_p;
		//dsurface_t     *in;
		//int             count;
		//int             i, j;
		//cSurface_t     *surface;
		//int             numVertexes;
		//static vec3_t   vertexes[SHADER_MAX_VERTEXES];
		//int             width, height;
		//int             shaderNum;
		//int             numIndexes;
		//static int      indexes[SHADER_MAX_INDEXES];
		//int            *index;
		//int            *index_p;
		
		
		int dsurface_t_size = (12 * 4 + 3 * 4 + 3 * 3 * 4 + 2 * 4);
		if(surfsLump.filelen % dsurface_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		int count = surfsLump.filelen / dsurface_t_size;
		
		ByteArrayReader surfReader = new ByteArrayReader(buf, surfsLump.fileofs, surfsLump.filelen);
		Engine.println("CollisionBspReader: loading " + count + " surfaces...");
		
		
		int drawVert_t_size = (3 * 4 + 2 * 4 + 2 * 4 + 3 * 4 + 4 * 4 + 4 * 4 + 3 * 4);
		if(vertsLump.filelen % drawVert_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}
		
		int index_t_size = 4;
		if(indexesLump.filelen % index_t_size != 0)
		{
			throw new RuntimeException("funny lump size");
		}

		// scan through all the surfaces, but only load patches,
		// not planar faces
		surfaces = new Surface[count];	
		for(int i = 0; i < count; i++)
		{
			int shaderNum = surfReader.readInt();
			int fogNum = surfReader.readInt();
			SurfaceType surfaceType = SurfaceType.values()[surfReader.readInt()];
			
			int firstVert = surfReader.readInt();
			int numVerts = surfReader.readInt();
		
			int firstIndex = surfReader.readInt();
			int numIndexes = surfReader.readInt();
		
			int lightmapNum = surfReader.readInt();
			int lightmapX = surfReader.readInt();
			int lightmapY = surfReader.readInt();
			int lightmapWidth = surfReader.readInt();
			int lightmapHeight = surfReader.readInt(); 
		
			for(int j = 0; j < (3 + 3 * 3); j++)
				surfReader.readFloat();
		
			int patchWidth = surfReader.readInt();
			int patchHeight = surfReader.readInt();
			
			if(numVerts == 0 || numIndexes == 0)
				continue;
			
			switch (surfaceType)
			{
				case TRIANGLE_SOUP:
					//Engine.println("loading triangle surface: vertices = " + numVerts + ", indices = " + numIndexes);
					
					Surface surface = surfaces[i] = new Surface();
					surface.type = surfaceType;
					surface.contents = shaders[shaderNum].contentFlags;
					surface.surfaceFlags = shaders[shaderNum].surfaceFlags;

					//surface.numVerts = numVerts;
					//surface.vertices = ByteBuffer.allocateDirect(numVerts * 3 * 4).order(ByteOrder.nativeOrder());
					
					ByteArrayReader drawVertReader = new ByteArrayReader(buf, vertsLump.fileofs + (firstVert * drawVert_t_size), vertsLump.filelen);
					
					surface.vertices = new ArrayList<Vector3f>();
					for(int j = 0; j < numVerts; j++)
					{
						Vector3f vertex = new Vector3f();
						
						vertex.x = drawVertReader.readFloat();
						vertex.y = drawVertReader.readFloat();
						vertex.z = drawVertReader.readFloat();
						
						surface.vertices.add(vertex);
						
						// skip the rest of the current drawVert_t
						for(int k = 0; k < (2 + 2 + 3 + 4 + 4 + 3); k++)
							drawVertReader.readFloat();
					}
					
					//surface.numIndices = numIndexes;
					//surface.indices = ByteBuffer.allocateDirect(numIndexes * 4).order(ByteOrder.nativeOrder());
					
					ByteArrayReader indexReader = new ByteArrayReader(buf, indexesLump.fileofs + (firstIndex * index_t_size), indexesLump.filelen);
					
					surface.indices = new ArrayList<Integer>();
					for(int j = 0; j < numIndexes; j++)
					{
						int index = indexReader.readInt();
						if(index < 0 || index >= numVerts)
						{
							throw new RuntimeException("bad index in trisoup surface");
						}
						
						surface.indices.add(new Integer(index));
					}
					//surface.indices.flip();
					
					//Engine.println("" + surface.indices);
					break;
			}
		}
	}
	
	
	public void addWorldBrushesToSimulation(List<CollisionShape> collisionShapes, DynamicsWorld dynamicsWorld) {
		// Engine.println("CollisionBspReader.addWorldBrushesToSimulation()");
		
		int totalVerts = 0;
		int	totalIndices = 0;
		
		int	checkcount = 1;

		// add brushes from all BSP leafs
		for (int i = 0; i < leafs.length; i++) {

			Leaf leaf = leafs[i];

			for (int j = 0; j < leaf.numLeafBrushes; j++) {

				int brushnum = leafBrushes[leaf.firstLeafBrush + j];

				Brush b = brushes[brushnum];

				if (b.checked) {
					continue;
				} else {
					b.checked = true;
				}

				if (b.sides.size() == 0) {
					// don't care about invalid brushes
					continue;
				}

				if ((b.contents & ContentFlags.SOLID) == 0) {
					// don't care about non-solid brushes
					continue;
				}

				List<Vector4f> planeEquations = new ArrayList<Vector4f>();

				for (BrushSide s : b.sides) {
					planeEquations.add(s.plane);
				}

				List<Vector3f> points = new ArrayList<Vector3f>();
				GeometryUtil.getVerticesFromPlaneEquations(planeEquations, points);

				// Engine.println("created " + points.size() +
				// " points for brush " + i);

				CollisionShape shape = new ConvexHullShape(points);

				collisionShapes.add(shape);

				// using motionstate is recommended, it provides interpolation
				// capabilities, and only synchronizes 'active' objects
				DefaultMotionState myMotionState = new DefaultMotionState();
				RigidBodyConstructionInfo rbInfo = new RigidBodyConstructionInfo(0, myMotionState, shape);
				RigidBody body = new RigidBody(rbInfo);

				// add the body to the dynamics world
				dynamicsWorld.addRigidBody(body);

				/*
				 * CollisionObject co = new CollisionObject();
				 * co.setCollisionShape(shape);
				 * co.setCollisionFlags(CollisionFlags.STATIC_OBJECT);
				 * dynamicsWorld.addCollisionObject(co);
				 */
			}
			
			
			for (int j = 0; j < leaf.numLeafSurfaces; j++) {

				int surfaceNum = leafSurfaces[leaf.firstLeafSurface + j];

				Surface surface = surfaces[surfaceNum];
				
				if(surface != null)
				{
					if (surface.checkcount == checkcount) {
						continue;
					} else {
						surface.checkcount = checkcount;
					}
					
					totalVerts += surface.vertices.size();
					totalIndices += surface.indices.size();
				}
			}
			
		}
		
		if(totalVerts == 0 || totalIndices == 0)
			return;
		
		ByteBuffer vertices = ByteBuffer.allocateDirect(totalVerts * 3 * 4).order(ByteOrder.nativeOrder());
		ByteBuffer indices = ByteBuffer.allocateDirect(totalIndices * 4).order(ByteOrder.nativeOrder());
		
		checkcount++;
		int numIndexes = 0;
		
		final int vertStride = 3 * 4;
		final int indexStride = 3 * 4;
		
		for (int i = 0; i < leafs.length; i++) {

			Leaf leaf = leafs[i];

			for (int j = 0; j < leaf.numLeafSurfaces; j++) {

				int surfaceNum = leafSurfaces[leaf.firstLeafSurface + j];

				Surface surface = surfaces[surfaceNum];

				if (surface != null) {
					
					if (surface.checkcount == checkcount) {
						continue;
					} else {
						surface.checkcount = checkcount;
					}
					
					//Engine.println("building BvhTriangleMeshShape: vertices = " + surface.vertices.size() + ", indices = " + surface.indices.size());
					
					//vertices = ByteBuffer.allocateDirect(surface.vertices.size() * 3 * 4).order(ByteOrder.nativeOrder());
					//indices = ByteBuffer.allocateDirect(surface.indices.size() * 4).order(ByteOrder.nativeOrder());

					for (Vector3f vertex : surface.vertices) {
						vertices.putFloat(vertex.x);
						vertices.putFloat(vertex.y);
						vertices.putFloat(vertex.z);
					}

					for (Integer index : surface.indices) {
						int newIndex = numIndexes + index.intValue();
						
						if(newIndex < 0 || newIndex >= totalVerts)
						{
							throw new RuntimeException("bad index in trisoup surface: index = " + newIndex + ", totalVerts = " + totalVerts);
						}
						
						indices.putInt(newIndex);
					}
					numIndexes += surface.vertices.size();
					
					/*
					TriangleIndexVertexArray indexVertexArrays = new TriangleIndexVertexArray(surface.indices.size() / 3, indices, indexStride,
							surface.vertices.size(), vertices, vertStride);
					
					BvhTriangleMeshShape trimeshShape = new BvhTriangleMeshShape(indexVertexArrays, true);
					collisionShapes.add(trimeshShape);

					// using motionstate is recommended, it provides
					// interpolation
					// capabilities, and only synchronizes 'active' objects
					DefaultMotionState myMotionState = new DefaultMotionState();
					RigidBodyConstructionInfo rbInfo = new RigidBodyConstructionInfo(0, myMotionState, trimeshShape);
					RigidBody body = new RigidBody(rbInfo);

					// add the body to the dynamics world
					dynamicsWorld.addRigidBody(body);
					*/
				}
			}
		}
		
		
		Engine.println("building BvhTriangleMeshShape for world: vertices = " + totalVerts + ", indices = " + totalIndices);
		//indices.flip();
		
		TriangleIndexVertexArray indexVertexArrays = new TriangleIndexVertexArray(totalIndices / 3, indices, indexStride,
				totalVerts, vertices, vertStride);
		
		BvhTriangleMeshShape trimeshShape = new BvhTriangleMeshShape(indexVertexArrays, true);
		collisionShapes.add(trimeshShape);

		// using motionstate is recommended, it provides
		// interpolation
		// capabilities, and only synchronizes 'active' objects
		DefaultMotionState myMotionState = new DefaultMotionState();
		RigidBodyConstructionInfo rbInfo = new RigidBodyConstructionInfo(0, myMotionState, trimeshShape);
		RigidBody body = new RigidBody(rbInfo);

		// add the body to the dynamics world
		dynamicsWorld.addRigidBody(body);
		
	}
}
