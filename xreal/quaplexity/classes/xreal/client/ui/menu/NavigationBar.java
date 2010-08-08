package xreal.client.ui.menu;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Vector;

import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.StackPanel;
import xreal.client.ui.VerticalAlignment;
import xreal.client.ui.StackPanel.Orientation;


/**
 * @author Robert Beckebans
 */
public class NavigationBar extends Component
{
	private StackPanel			stackPanel;
	private NavigationButton	select;
	private NavigationButton	back;
	
	@SuppressWarnings("rawtypes")
	public NavigationBar()
	{
		horizontalAlignment = HorizontalAlignment.Stretch;
		verticalAlignment = VerticalAlignment.Bottom;
		
		//orientation = Orientation.Horizontal;
		
		backgroundImage = new Image("white");
		backgroundImage.color.red = 0.0f;
		backgroundImage.color.green = 0.0f;
		backgroundImage.color.blue = 0.0f;
		backgroundImage.color.alpha = 0.5f;
		
		stackPanel = new StackPanel();
		stackPanel.orientation = Orientation.Horizontal;
		stackPanel.margin.left = 33;
		
		add("SELECT", "ui/keyboard_keys/standard_104/enter.png");
		add("BACK", "ui/keyboard_keys/standard_104/esc.png");
		
		addChild(stackPanel);
	}
	
	/**
	 * Remove buttons from navigation bar.
	 */
	public void clear()
	{
		stackPanel.clearChildren();
	}
	
	
	public void add(String labelText, String materialName)
	{
		NavigationButton button = new NavigationButton(materialName, labelText);
	    button.margin.left = 10;
	    
	    stackPanel.addChild(button);
	}
}
