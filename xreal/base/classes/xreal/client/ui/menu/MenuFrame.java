package xreal.client.ui.menu;

import xreal.Engine;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.Container;
import xreal.client.ui.Cursor;
import xreal.client.ui.Rectangle;
import xreal.client.ui.UserInterface;
import xreal.client.ui.event.Event;
import xreal.client.ui.event.MouseEvent;
import xreal.client.ui.event.MouseMotionListener;

public abstract class MenuFrame extends Container implements MouseMotionListener {
	
	protected Font fontVera;
	protected Font fontVeraSe;
	protected Font fontVeraBold;
	protected Font fontVeraSerifBold;
	
	protected boolean wrapAround;
	protected boolean fullscreen;
	protected boolean showlogo;
	
	protected MenuFrame() {
		
		bounds.setBounds(0, 0, UserInterface.SCREEN_WIDTH, UserInterface.SCREEN_HEIGHT);
		
		children.add(UserInterface.getCursor());
		
		fontVera = Renderer.registerFont("fonts/Vera.ttf", 48);
		fontVeraSe = Renderer.registerFont("fonts/VeraSe.ttf", 48);
		fontVeraBold = Renderer.registerFont("fonts/VeraBd.ttf", 48);
		fontVeraSerifBold = Renderer.registerFont("fonts/VeraSeBd.ttf", 48);
	}
	
	@Override
	public void mouseMoved(MouseEvent e) {
		//Engine.println("MenuFrame.mouseMoved()");
		
		Cursor cursor = UserInterface.getCursor();
		
		// region test the active menu items
		for(Component m : children)
		{
			if(m instanceof Cursor)
				continue;
			
			if(m.hasFlags(Component.QMF_GRAYED | Component.QMF_INACTIVE))
				continue;

			if(!m.contains(cursor.getX(), cursor.getY()))
			{
				// cursor out of item bounds
				m.delFlags(QMF_HASMOUSEFOCUS);
				continue;
			}

			// set focus to item at cursor
			//if(uis.activemenu->cursor != i)
			{
				//Menu_SetCursor(uis.activemenu, i);
				
				if(!m.hasFlags(Component.QMF_GRAYED | Component.QMF_INACTIVE))
				{
					Engine.println("item has focus at x = " + cursor.getX() + ", y = " + cursor.getY() + ", component bounds = " + m.getBounds());
					
					//m->cursor_prev = m->cursor;
					//m->cursor = cursor;

					//Menu_CursorMoved(m);
					
					m.addFlags(QMF_HASMOUSEFOCUS);
				}
				
				//((menucommon_s *) (uis.activemenu->items[uis.activemenu->cursor_prev]))->flags &= ~QMF_HASMOUSEFOCUS;

				/*
				if(!(((menucommon_s *) (uis.activemenu->items[uis.activemenu->cursor]))->flags & QMF_SILENT))
				{
					trap_S_StartLocalSound(menu_move_sound, CHAN_LOCAL_SOUND);
				}
				*/
			}

			//((menucommon_s *) (uis.activemenu->items[uis.activemenu->cursor]))->flags |= QMF_HASMOUSEFOCUS;
			//return;
		}
	}

	@Override
	public void processEvent(Event e) {
		if (e instanceof MouseEvent) {
			mouseMoved((MouseEvent) e);
		}
		
		super.processEvent(e);
	}
}
