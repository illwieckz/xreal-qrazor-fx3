package xreal.client.ui;

import java.util.ArrayList;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import xreal.client.ui.event.Event;
import xreal.client.ui.event.EventListener;
import xreal.client.ui.event.MouseMotionListener;
import xreal.server.game.GameEntity;

/**
 * 
 * @author Robert Beckebans
 */
public class Component implements EventListener {

	protected Rectangle bounds = new Rectangle(0, 0, 640, 480);
	
	@Override
	public boolean consumeEvent(Event e) {
		return false;
	}
	
	public Rectangle getBounds() {
		return bounds;
	}

	public void render() {
	}
	
	/**
	 * Adjusted for resolution and screen aspect ratio
	 */
	/*
	public void adjustFrom640() {
		bounds.x *= screenXScale;
		bounds.y *= screenYScale;
		bounds.width *= screenXScale;
		bounds.height *= screenYScale;
	}
	*/
}
