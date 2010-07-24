package xreal.client.ui.border;

import xreal.Color;
import xreal.client.ui.Component;

public abstract class Border
{
	public Color	borderColor	= Color.White;

	public abstract void paintBorder(float x, float y, float width, float height);

	public abstract void paintBorder(Component c, float x, float y, float width, float height);
}
