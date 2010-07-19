package xreal.client.ui.event;

import java.util.EventObject;


/**
 * 
 * @author Robert Beckebans
 */
public class Event extends EventObject {

	public Event(Object source) {
		super(source);
	}

	/*
	private boolean consumed = false;
	
	public void consume() {
		consumed = true;
	}
	
	public boolean isConsumed() {
		return consumed;
	}
	*/
}
