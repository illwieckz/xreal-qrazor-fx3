package xreal.client.ui;

/**
 * 
 * @author Robert Beckebans
 */
public class Rectangle {

	/**
	 * The x coordinate of the upper-left corner of the Rectangle.
	 */
	public float x;

	/**
	 * The y coordinate of the upper-left corner of the Rectangle.
	 */
	public float y;

	/**
	 * The width of the Rectangle.
	 */
	public float width;

	/**
	 * The height of the Rectangle.
	 */
	public float height;

	public Rectangle(float x, float y, float width, float height) {
		super();
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;
	}

	public Rectangle(Rectangle r) {
		super();
		this.x = r.x;
		this.y = r.y;
		this.width = r.width;
		this.height = r.height;
	}

	public void setBounds(float x, float y, float width, float height) {
		this.x = x;
		this.y = y;
		this.width = width;
		this.height = height;
	}

	public void setBounds(Rectangle r) {
		this.x = r.x;
		this.y = r.y;
		this.width = r.width;
		this.height = r.height;
	}
	
	public void setLocation(float x, float y) {
		this.x = x;
		this.y = y;
	}
	
	public void setCenter(float x, float y) {
		this.x = x - width / 2;
		this.y = y - height / 2;
	}
	
	public void setXCenter(float x) {
		this.x = x - width / 2;
	}
	
	public void setYCenter(float y) {
		this.y = y - height / 2;
	}

	public boolean contains(float x, float y) {

		// construct lower right corner
		float x1 = this.x + Math.max(0, width);
		float y1 = this.y + Math.max(0, height);

		return (x >= this.x) && (y >= this.y) && (x < x1) && (y < y1);
	}
	
	@Override
	public String toString() {
		return "[" + x + ", " + y + ", " + width + ", " + height + "]";
	}
}
