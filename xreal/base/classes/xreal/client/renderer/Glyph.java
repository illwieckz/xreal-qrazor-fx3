package xreal.client.renderer;

public class Glyph {
	/**
	 * number of scan lines
	 */
	public int height;

	/**
	 * top of glyph in buffer
	 */
	public int top;

	/**
	 * bottom of glyph in buffer
	 */
	public int bottom;

	/**
	 * width for copying
	 */
	public int pitch;

	/**
	 * x adjustment
	 */
	public int xSkip;

	/**
	 * width of actual image
	 */
	public int imageWidth;

	/**
	 * height of actual image
	 */
	public int imageHeight;

	/**
	 * x offset in image where glyph starts
	 */
	public float s;

	/**
	 * y offset in image where glyph starts
	 */
	public float t;

	public float s2;
	public float t2;

	/**
	 * handle to the shader with the glyph
	 */
	public int glyph;

	public String materialName;

	public Glyph(int height, int top, int bottom, int pitch, int xSkip, int imageWidth, int imageHeight, float s, float t, float s2, float t2, int glyph,
			String materialName) {
		super();
		this.height = height;
		this.top = top;
		this.bottom = bottom;
		this.pitch = pitch;
		this.xSkip = xSkip;
		this.imageWidth = imageWidth;
		this.imageHeight = imageHeight;
		this.s = s;
		this.t = t;
		this.s2 = s2;
		this.t2 = t2;
		this.glyph = glyph;
		this.materialName = materialName;
	}

	@Override
	public String toString() {
		return "Glyph:" +
		" height = " + height +
		" top = " + top +
		" bottom = " + bottom +
		" pitch = " + pitch +
		" xSkip = " + xSkip +
		" imageWidth = " + imageWidth +
		" imageHeight = " + imageHeight +
		" s = " + s +
		" t = " + t +
		" s2 = " + s2 +
		" t2 = " + t2 +
		" glyph = " + glyph +
		" materialName = " + materialName;
	}
}
