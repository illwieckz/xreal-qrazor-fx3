package xreal.client.ui.event;

import xreal.client.ui.Component;

/**
 * 
 * @author Robert Beckebans
 */
public class ComponentEvent extends Event {

	private Component component;
	
	protected ComponentEvent(Component source) {
		super(source);
		this.component = source;
	}
	
	public Component getComponent() {
		return component;
	}
}
