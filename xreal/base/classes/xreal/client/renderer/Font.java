package xreal.client.renderer;

import xreal.Engine;

public class Font {
	
	public static final int GLYPH_START = 0;
	public static final int GLYPH_END = 255;
	public static final int GLYPH_CHARSTART = 32;
	public static final int GLYPH_CHAREND = 127;
	public static final int GLYPHS_PER_FONT = GLYPH_END - GLYPH_START + 1;

	private Glyph glyphs[];
	private float glyphScale;
	private String name;
	
	/**
	 * Usually only called by the engine.
	 * 
	 * @param glyphs
	 * @param glyphScale
	 * @param name
	 */
	public Font(Glyph[] glyphs, float glyphScale, String name) {
		super();
		this.glyphs = glyphs;
		this.glyphScale = glyphScale;
		this.name = name;
		
		/*
		for debugging
		Engine.println("Font(glyphScale = " + glyphScale + ", name = " + name);
		for(int i = 0; i < GLYPHS_PER_FONT; i++) {
			Engine.println(glyphs[i].toString());
		}
		*/
	}
}
