package xreal.client.ui;

import java.util.LinkedHashSet;
import java.util.Set;

import xreal.client.ui.event.Event;

public class Container extends Component {

	protected Set<Component> children = new LinkedHashSet<Component>();

	protected void fireEvent(Event e) {

		for (Component l : children) {
			if (l.consumeEvent(e))
				break;
		}
	}

	public void render() {
		
		for (Component c : children) {
			c.render();
		}
	}
}
