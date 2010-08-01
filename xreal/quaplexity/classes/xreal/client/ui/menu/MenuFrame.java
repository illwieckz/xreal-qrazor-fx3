package xreal.client.ui.menu;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCode;
import xreal.client.SoundChannel;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.Cursor;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.LinearFocusTraversalPolicy;
import xreal.client.ui.Rectangle;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.VerticalAlignment;
import xreal.client.ui.event.Event;
import xreal.client.ui.event.FocusEvent;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.KeyListener;
import xreal.client.ui.event.MouseEvent;
import xreal.client.ui.event.MouseMotionListener;
import xreal.client.ui.event.FocusEvent.FocusType;


/**
 * @author Robert Beckebans
 */
public class MenuFrame extends Component implements MouseMotionListener, KeyListener
{
	protected Font		fontVera;
	protected Font		fontVeraSe;
	protected Font		fontVeraBold;
	protected Font		fontVeraSerifBold;

	protected int		soundIn;
	protected int		soundMove;
	protected int		soundOut;
	protected int		soundBuzz;

	protected boolean	wrapAround	= true;
	protected boolean	fullscreen;

	NavigationBar		navigationBar;
	
	private Component	cursor;
	private Component	cursorPrev;
	

	public boolean isFullscreen()
	{
		return fullscreen;
	}

	protected boolean	showlogo;

	protected MenuFrame(String backgroundImageName)
	{
		bounds.x = 0;
		bounds.y = 0;
		bounds.width = width = UserInterface.SCREEN_WIDTH;
		bounds.height = height = UserInterface.SCREEN_HEIGHT;
		
		// background image
		backgroundImage = new Image(backgroundImageName);

		Cursor cursor = UserInterface.getCursor();
		//cursor.addMouseMotionListener(this);
		addChild(cursor);

		fontVera = Renderer.registerFont("fonts/Vera.ttf", 48);
		fontVeraSe = Renderer.registerFont("fonts/VeraSe.ttf", 48);
		fontVeraBold = Renderer.registerFont("fonts/VeraBd.ttf", 48);
		fontVeraSerifBold = Renderer.registerFont("fonts/VeraSeBd.ttf", 48);

		soundIn = Client.registerSound("sound/misc/menu1.wav");
		soundMove = Client.registerSound("sound/misc/menu2.wav");
		soundOut = Client.registerSound("sound/misc/menu3.wav");
		soundBuzz = Client.registerSound("sound/misc/menu4.wav");
		
		// navigation bar
		navigationBar = new NavigationBar();
		navigationBar.margin.bottom = 20;
		
		addChild(navigationBar);
	}

	@Override
	public void mouseMoved(MouseEvent e)
	{
		//Engine.println("MenuFrame.mouseMoved()");

		Cursor cursor = UserInterface.getCursor();

		// region test the active menu items
		/*
		for(Component c : children)
		{
			if(c instanceof Cursor)
				continue;

			if(c.grayed || !c.active)
				continue;

			if(!c.contains(cursor.getX(), cursor.getY()))
			{
				// cursor out of item bounds
				c.hasMouseFocus = false;
				continue;
			}

			// set focus to item at cursor
			// if(uis.activemenu->cursor != i)
			{
				// Menu_SetCursor(uis.activemenu, i);

				if(!c.grayed || c.active)
				{
					// Engine.println("item has focus at x = " + cursor.getX() + ", y = " + cursor.getY() +
					// ", component bounds = " + c.getBounds());

					// m->cursor_prev = m->cursor;
					// m->cursor = cursor;

					// Menu_CursorMoved(m);

					c.hasMouseFocus = true;
				}

				// ((menucommon_s *) (uis.activemenu->items[uis.activemenu->cursor_prev]))->flags &= ~QMF_HASMOUSEFOCUS;

				if(!c.silent)
				{
					// Client.startLocalSound(soundMove, SoundChannel.LOCAL_SOUND);
				}
			}

			// ((menucommon_s *) (uis.activemenu->items[uis.activemenu->cursor]))->flags |= QMF_HASMOUSEFOCUS;
			return;
		}
		*/
	}

	@Override
	public void keyPressed(KeyEvent e)
	{
		KeyCode key = e.getKey();
		
		if(!e.isDown())
			return;
		
		//Engine.println("MenuFrame.keyPressed(event = " + e + ")");
		
		if(cursor != null)
		{
			if(cursor.isFocusOwner() && cursor instanceof KeyListener)
			{
				((KeyListener)cursor).keyPressed(e);
				
				if(e.isConsumed())
					return;
			}
		}
		
		switch(key)
		{
			case F12:
				Engine.sendConsoleCommand(Engine.EXEC_APPEND, "screenshotJPEG\n");
				break;
	
			case KP_UPARROW:
			case UPARROW:
			case XBOX360_DPAD_UP:
				adjustCursorPrev();
				break;

			case TAB:
			case KP_DOWNARROW:
			case DOWNARROW:
			case XBOX360_DPAD_DOWN:
				adjustCursorNext();
				break;
		}
	}
	
	protected void setCursor(Component c)
	{
		if(cursor == c)
			return;
		
		cursorPrev = cursor;
		cursor = c; //children.indexOf(c);
		
		cursorMoved();
	}
	
	protected void setCursorOrder(Vector<Component> order)
	{
		setFocusTraversalPolicy(new LinearFocusTraversalPolicy(order));
	}
	
	void adjustCursorNext()
	{
		Component c, start;
		c = start = getComponentAfter(this, cursor);
		do
		{
			if(c.isFocusable() && c.active && !c.grayed)
				break;
			
			c = getComponentAfter(this, c);
		}
		while(c != null && c != start);
			
		if(cursor != c)
		{
			setCursor(c);
		}
	}
	
	void adjustCursorPrev()
	{
		Component c, start;
		c = start = getComponentBefore(this, cursor);
		do
		{
			if(c.isFocusable() && c.active && !c.grayed)
				break;
			
			c = getComponentBefore(this, c);
		}
		while(c != null && c != start);
			
		if(cursor != c)
		{
			setCursor(c);
		}
	}
	
	private void cursorMoved()
	{
		if(cursorPrev == cursor)
			return;
		
		if(cursorPrev != null)
		{
			cursorPrev.focusLost(new FocusEvent(this, FocusType.LOST));
		}
		
		if(cursor != null)
		{
			cursor.focusGained(new FocusEvent(this, FocusType.GAINED));
		}
	}

	@Override
	public void keyReleased(KeyEvent e)
	{
		// Engine.println("MenuFrame.keyReleased()");
	}

	@Override
	public void processEvent(Event e)
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

		// super.processEvent(e);
	}
}
