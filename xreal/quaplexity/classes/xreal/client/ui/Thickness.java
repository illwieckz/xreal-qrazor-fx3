package xreal.client.ui;

/**
 * @author Robert Beckebans
 */
public class Thickness
{
	public float	left;
	public float	top;
	public float	right;
	public float	bottom;
	
	public Thickness()
	{
		this.left = 0;
		this.top = 0;
		this.right = 0;
		this.bottom = 0;
	}
	
	public Thickness(float size)
	{
		super();
		this.left = size;
		this.top = size;
		this.right = size;
		this.bottom = size;
	}
	
	public Thickness(float left, float top, float right, float bottom)
	{
		super();
		this.left = left;
		this.top = top;
		this.right = right;
		this.bottom = bottom;
	}
	
	public void set(float size)
	{
		this.left = size;
		this.top = size;
		this.right = size;
		this.bottom = size;
	}
}
