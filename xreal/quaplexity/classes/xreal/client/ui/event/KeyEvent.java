package xreal.client.ui.event;

import xreal.client.KeyCode;
import xreal.client.ui.Component;


/**
 * SE_KEY system event
 * 
 * @author Robert Beckebans
 */
public class KeyEvent extends InputEvent
{
	private KeyCode	key;
	private boolean	down;
	protected boolean charEvent;

	public KeyEvent(Component source, int time, KeyCode key, boolean down)
	{
		super(source, time, 0);
		this.key = key;
		this.down = down;
		this.charEvent = false;
	}

	public KeyCode getKey()
	{
		return key;
	}

	/**
	 * Returns a String describing the keyCode, such as "HOME", "F1" or "A".
	 */
	public String getKeyText()
	{
		return key.getText();
	}

	/**
	 * Returns the integer keyCode associated with the key in this event.
	 */
	public int getKeyCode()
	{
		return key.getCode();
	}

	public boolean isDown()
	{
		return down;
	}

	@Override
	public String toString()
	{
		return "[code = '" + key.getCode() + "', text = '" + key.getText() + "', down = '" + down + "'";
	}

	/**
	 * 
	 * @return True if the key code was  or'ed in K_CHAR_FLAG 1024
	 */
	public boolean isCharEvent()
	{
		return charEvent;
	}
}
