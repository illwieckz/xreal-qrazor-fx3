package xreal.client.ui.menu;

import xreal.Color;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.Image;
import xreal.client.ui.Label;
import xreal.client.ui.StackPanel;
import xreal.client.ui.VerticalAlignment;

public class NavigationButton extends StackPanel
{
	private Image	image;
	private Label	label;
	
	public NavigationButton(String imageName, String labelText)
	{
		orientation = Orientation.Horizontal;
		
		image = new Image(imageName);
		image.width = 24;
		image.height = 24;
		image.margin.set(4);
		image.verticalAlignment = VerticalAlignment.Center;
		
		Component canvas = new Component();
		canvas.addChild(image);
		
		label = new Label(labelText);
		label.height = 32;
		label.textBlock.font = Renderer.registerFont("fonts/Vera.ttf", 48);
		label.textBlock.fontSize = 20;
		label.textBlock.color.set(Color.MdGrey);
		
		addChild(canvas);
		addChild(label);
	}
}
