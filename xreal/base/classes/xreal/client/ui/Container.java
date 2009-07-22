package xreal.client.ui;

import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.Set;
import java.util.Vector;

import xreal.client.ui.event.Event;

public class Container extends Component {

	//protected Set<Component> children = new LinkedHashSet<Component>();
	protected Vector<Component> children = new Vector<Component>();

	protected void fireEvent(Event e) {
		
		processEvent(e);

		for (Component l : children) {
			l.processEvent(e);
		}
	}

	public void render() {
		
		for (Component c : children) {
			c.render();
		}
	}
}
