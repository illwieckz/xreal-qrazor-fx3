package xreal.client.ui.event;

import xreal.client.ui.Component;

/**
 * 
 * @author Robert Beckebans
 */
public abstract class InputEvent extends ComponentEvent
{

	private int	time;
	private int	modifiers;

	public InputEvent(Component source, int time, int modifiers)
	{
		super(source);
		this.time = time;
		this.modifiers = modifiers;
	}

	public int getWhen()
	{
		return time;
	}

	public int getModifiers()
	{
		return modifiers;
	}
}
