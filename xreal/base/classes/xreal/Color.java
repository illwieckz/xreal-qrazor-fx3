package xreal;

/**
 * 
 * @author Robert Beckebans
 */
public class Color {

	public float red;
	public float green;
	public float blue;
	public float alpha;

	public Color(float red, float green, float blue, float alpha) {
		super();
		this.red = red;
		this.green = green;
		this.blue = blue;
		this.alpha = alpha;
	}

	public Color(float red, float green, float blue) {
		super();
		this.red = red;
		this.green = green;
		this.blue = blue;
		this.alpha = 1;
	}
	
	public void set(float red, float green, float blue, float alpha) {
		this.red = red;
		this.green = green;
		this.blue = blue;
		this.alpha = alpha;
	}
	
	public void set(float red, float green, float blue) {
		this.red = red;
		this.green = green;
		this.blue = blue;
		this.alpha = 1;
	}
	
	public void set(Color color) {
		this.red = color.red;
		this.green = color.green;
		this.blue = color.blue;
		this.alpha = color.alpha;
	}
	
	public void setRGB(Color color) {
		this.red = color.red;
		this.green = color.green;
		this.blue = color.blue;
	}

	float normalize() {
		float max;

		max = red;
		if (green > max) {
			max = green;
		}
		if (blue > max) {
			max = blue;
		}

		if (max <= 0) {
			red = 0;
			green = 0;
			blue = 0;
		} else {
			red /= max;
			green /= max;
			blue /= max;
		}

		return max;
	}

	void clampLDR() {

		if (red < 0)
			red = 0;

		if (green < 0)
			green = 0;

		if (blue < 0)
			blue = 0;

		if (alpha < 0)
			alpha = 0;

		if (red > 1)
			red = 1;

		if (green > 1)
			green = 1;

		if (blue > 1)
			blue = 1;

		if (alpha > 1)
			alpha = 1;
	}
	
	void clampHDR() {

		if (red < 0)
			red = 0;

		if (green < 0)
			green = 0;

		if (blue < 0)
			blue = 0;

		if (alpha < 0)
			alpha = 0;
	}

	/*
	@Override
	public boolean equals(Object obj) {
		Color c = (Color) obj;
		
		return (this.red == c.red && this.green == c.green && this.blue == c.blue && this.alpha == c.alpha);
	}
	*/

	@Override
	public String toString() {
		return "(" + red + ", " + green + ", " + blue + ", " + alpha + ")";
	}

	public static final Color Black = new Color(0, 0, 0, 1);
	public static final Color Red = new Color(1, 0, 0, 1);
	public static final Color Green = new Color(0, 1, 0, 1);
	public static final Color Blue = new Color(0, 0, 1, 1);
	public static final Color Yellow = new Color(1, 1, 0, 1);
	public static final Color Magenta = new Color(1, 0, 1, 1);
	public static final Color Cyan = new Color(0, 1, 1, 1);
	public static final Color White = new Color(1, 1, 1, 1);
	public static final Color LtGrey = new Color(0.75f, 0.75f, 0.75f, 1);
	public static final Color MdGrey = new Color(0.5f, 0.5f, 0.5f, 1);
	public static final Color DkGrey = new Color(0.25f, 0.25f, 0.25f, 1);
}
