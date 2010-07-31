package xreal.client.ui.event;

import xreal.client.ui.Component;

/**
 * @author Robert Beckebans
 */
public class FocusEvent extends ComponentEvent
{
	public enum FocusType
	{
		FIRST,
		GAINED,
		LOST,
		LAST
	}
	
	private FocusType type;
	
	public FocusEvent(Component source, FocusType type)
	{
		super(source);
	}
	
	public FocusType getType()
	{
		return type;
	}
}
