package xreal.client.ui.menu;

import xreal.Color;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Image;
import xreal.client.ui.Label;

public class MenuTitle extends Label
{
	public MenuTitle(String title)
	{
		super();
		
		text = title;
		
		height = 32;
		margin.bottom = 26;
		textBlock.font = Renderer.registerFont("fonts/FreeSansBoldOblique.ttf", 48);
		textBlock.fontSize = 34;
		textBlock.color.set(Color.LtGrey);
		backgroundImage = new Image("white");
		backgroundImage.color.set(0.0f, 0.0f, 0.0f, 0.5f);
	}
}
