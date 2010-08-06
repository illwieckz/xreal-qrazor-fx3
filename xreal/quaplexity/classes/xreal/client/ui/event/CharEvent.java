package xreal.client.ui.event;

import xreal.client.KeyCode;
import xreal.client.ui.Component;

/**
 * SE_CHAR system event
 * 
 * @author Robert Beckebans
 */
public class CharEvent extends KeyEvent
{
	public CharEvent(Component source, int time, KeyCode key, boolean down)
	{
		super(source, time, key, down);
		charEvent = true;
	}

}
