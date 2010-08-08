package xreal.client.ui.menu;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import xreal.Color;
import xreal.Engine;
import xreal.client.Client;
import xreal.client.KeyCode;
import xreal.client.SoundChannel;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Button;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.Label;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.VerticalAlignment;
import xreal.client.ui.event.FocusEvent;
import xreal.client.ui.event.KeyEvent;
import xreal.client.ui.event.FocusEvent.FocusType;


/**
 * @author Robert Beckebans
 */
public class OptionsMenu extends MenuFrame 
{
	Label						title;
	StackPanel					stackPanel;
	Button						audioButton;
	Button						graphicsButton;
	Button						gameSettingsButton;
	Button						controlsSetupButton;
	Button						keyboardSetupButton;
	
	public OptionsMenu() 
	{
		super("menuback");
		
		backgroundImage.color.set(Color.LtGrey);
		
		fullscreen = true;
		
		title = new MenuTitle("OPTIONS");
		
		audioButton = new MenuButton("AUDIO")
		{
			public void keyPressed(KeyEvent e)
			{
				KeyCode key = e.getKey();
				
				switch(key)
				{
					case ENTER:
					case MOUSE1:
					case XBOX360_A:
						UserInterface.pushMenu(new OptionsMenu_Audio());
						e.consume();
						break;
				}
			}
		};
		
		graphicsButton = new MenuButton("GRAPHICS");
		gameSettingsButton = new MenuButton("GAME");
		controlsSetupButton = new MenuButton("CONTROLS");
		keyboardSetupButton = new MenuButton("KEYBOARD")
		{
			/*
			public void keyPressed(KeyEvent e)
			{
				if(!e.isDown())
					return;
			
				KeyCode key = e.getKey();
				
				Engine.println("keyboardSetupButton.keyPressed(event = " + e + ")");
				
				switch(key)
				{
					case ENTER:
					case MOUSE1:
					case XBOX360_A:
						UserInterface.pushMenu(new QuitMenu());
						e.consume();
						break;
				}
			}
			*/
		};
		
		stackPanel = new StackPanel();
		stackPanel.horizontalAlignment = HorizontalAlignment.Left;
		stackPanel.verticalAlignment = VerticalAlignment.Bottom;
		stackPanel.margin.bottom = 100;
		stackPanel.margin.left = 43;
		
		stackPanel.addChild(title);
		stackPanel.addChild(audioButton);
		stackPanel.addChild(graphicsButton);
		stackPanel.addChild(gameSettingsButton);
		stackPanel.addChild(controlsSetupButton);
		stackPanel.addChild(keyboardSetupButton);
		
		addChild(stackPanel);
		
		
		Vector<Component> order = new Vector<Component>();
		order.add(audioButton);
		order.add(graphicsButton);
		order.add(gameSettingsButton);
		order.add(controlsSetupButton);
		order.add(keyboardSetupButton);
		setCursorOrder(order);
		
		setCursor(audioButton);
	}
	
	@Override
	protected void updateNavigationBarPC()
	{
		navigationBar.clear();
		navigationBar.add("SELECT", "ui/keyboard_keys/standard_104/enter.png");
		navigationBar.add("BACK", "ui/keyboard_keys/standard_104/esc.png");
		
		super.updateNavigationBarPC();
	}
	
	@Override
	protected void updateNavigationBar360()
	{
		navigationBar.clear();
		navigationBar.add("SELECT", "ui/xbox360/xna/buttons/xboxControllerButtonA.png");
		navigationBar.add("BACK", "ui/xbox360/xna/buttons/xboxControllerButtonB.png");
		
		super.updateNavigationBar360();
	}
}

