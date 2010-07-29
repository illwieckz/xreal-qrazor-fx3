package xreal.client.ui;

import xreal.Engine;
import xreal.client.ui.event.Event;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.KeyListener;
import xreal.client.ui.event.MouseEvent;
import xreal.client.ui.event.MouseMotionListener;


/**
 * @author Robert Beckebans
 */
public abstract class AbstractButton extends Label implements MouseMotionListener, KeyListener
{

	@Override
	public void mouseMoved(MouseEvent e)
	{
		// Engine.println("AbstractButton.mouseMoved()");

		MouseMotionListener listeners[] = getMouseMotionListeners();

		for(MouseMotionListener l : listeners)
		{
			l.mouseMoved(e);
		}
	}

	@Override
	public void keyPressed(KeyEvent e)
	{
		// Engine.println("AbstractButton.keyPressed()");

		KeyListener listeners[] = getKeyListeners();

		for(KeyListener l : listeners)
		{
			l.keyPressed(e);
		}
	}

	@Override
	public void keyReleased(KeyEvent e)
	{
		// Engine.println("AbstractButton.keyReleased()");

		KeyListener listeners[] = getKeyListeners();

		for(KeyListener l : listeners)
		{
			l.keyReleased(e);
		}
	}

	@Override
	public void processEvent(Event e)
	{
		if(hasMouseFocus)
		{
			if(e instanceof MouseEvent)
			{
				mouseMoved((MouseEvent) e);
			}
			else if(e instanceof KeyEvent)
			{
				if(((KeyEvent) e).isDown())
				{
					keyPressed((KeyEvent) e);
				}
				else
				{
					keyReleased((KeyEvent) e);
				}
			}
		}

		super.processEvent(e);
	}

}
