package xreal.client.ui.event;

import xreal.client.ui.Component;

/**
 * 
 * @author Robert Beckebans
 */
public class MouseEvent extends InputEvent
{

	private int	x;
	private int	y;

	public MouseEvent(Component source, int time, int modifiers, int x, int y)
	{
		super(source, time, modifiers);
		this.x = x;
		this.y = y;
	}

	public int getX()
	{
		return x;
	}

	public int getY()
	{
		return y;
	}
}
