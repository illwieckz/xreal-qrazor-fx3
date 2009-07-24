package xreal.client.ui.border;

import xreal.client.ui.Component;

public abstract class Border {

	public abstract void paintBorder(float x, float y, float width, float height);
	
	public abstract void paintBorder(Component c, float x, float y, float width, float height);
}
