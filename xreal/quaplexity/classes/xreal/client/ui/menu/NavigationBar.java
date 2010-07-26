package xreal.client.ui.menu;

import xreal.CVars;
import xreal.Color;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.StackPanel;
import xreal.client.ui.StackPanel.Orientation;

public class NavigationBar extends Component
{
	Image				backgroundImage;
	StackPanel			stackPanel;
	NavigationButton	select;
	NavigationButton	back;
	
	public NavigationBar()
	{
		horizontalAlignment = HorizontalAlignment.Stretch;
		
		backgroundImage = new Image("white");
		backgroundImage.color.red = 0.1f;
		backgroundImage.color.green = 0.1f;
		backgroundImage.color.blue = 0.1f;
		backgroundImage.color.alpha = 0.8f;
		
		stackPanel = new StackPanel();
		stackPanel.orientation = Orientation.Horizontal;
		
		select = new NavigationButton("ui/keyboard_keys/standard_104/enter.png", "SELECT");
		select.margin.left = 10;
		
		back = new NavigationButton("ui/keyboard_keys/standard_104/esc.png", "BACK");
		back.margin.left = 10;
		
		stackPanel.addChild(select);
		stackPanel.addChild(back);
		
		this.addChild(stackPanel);
	}
	
	@Override
	public void render()
	{
		backgroundImage.setBounds(bounds);
		backgroundImage.render();
		
		super.render();
	}
}
