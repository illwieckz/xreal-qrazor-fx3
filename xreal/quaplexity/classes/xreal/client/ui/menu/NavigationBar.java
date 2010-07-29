package xreal.client.ui.menu;

import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.StackPanel;
import xreal.client.ui.VerticalAlignment;


/**
 * @author Robert Beckebans
 */
public class NavigationBar extends StackPanel
{
	Image				backgroundImage;
	StackPanel			stackPanel;
	NavigationButton	select;
	NavigationButton	back;
	
	public NavigationBar()
	{
		horizontalAlignment = HorizontalAlignment.Stretch;
		verticalAlignment = VerticalAlignment.Bottom;
		
		orientation = Orientation.Horizontal;
		
		backgroundImage = new Image("white");
		backgroundImage.color.red = 0.1f;
		backgroundImage.color.green = 0.1f;
		backgroundImage.color.blue = 0.1f;
		backgroundImage.color.alpha = 0.8f;
		
		//stackPanel = new StackPanel();
		//stackPanel.orientation = Orientation.Horizontal;
		
		select = new NavigationButton("ui/keyboard_keys/standard_104/enter.png", "SELECT");
		select.margin.left = 30;
		
		back = new NavigationButton("ui/keyboard_keys/standard_104/esc.png", "BACK");
		back.margin.left = 10;
		
		addChild(select);
		addChild(back);
		
		//this.addChild(stackPanel);
	}
	
	@Override
	public void render()
	{
		backgroundImage.setBounds(bounds);
		backgroundImage.render();
		
		super.render();
	}
}
