package xreal.client.ui.menu;

import java.util.HashMap;
import java.util.Map;

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
	private Image				backgroundImage;
	private StackPanel			stackPanel;
	private NavigationButton	select;
	private NavigationButton	back;
	
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
		
		Map<String, String> navButtons = new HashMap<String, String>();
		navButtons.put("SELECT", "ui/keyboard_keys/standard_104/enter.png");
		navButtons.put("BACK", "ui/keyboard_keys/standard_104/esc.png");
		set(navButtons);
		
		addChild(stackPanel);
	}
	
	public void set(Map<String, String> buttons)
	{
		stackPanel.clearChildren();
		
		for (Map.Entry<String, String> e : buttons.entrySet())
		{
		    System.out.println(e.getKey() + ": " + e.getValue());
		    
		    NavigationButton button = new NavigationButton(e.getValue(), e.getKey());
		    button.margin.left = 10;
		    
		    stackPanel.addChild(button);
		}
	}
	
	@Override
	public void render()
	{
		backgroundImage.setBounds(bounds);
		backgroundImage.render();
		
		super.render();
	}
}
