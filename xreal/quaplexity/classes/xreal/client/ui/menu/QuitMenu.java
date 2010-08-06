package xreal.client.ui.menu;

import java.util.HashMap;
import java.util.Map;
import java.util.Vector;

import xreal.Color;
import xreal.Engine;
import xreal.client.KeyCode;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Button;
import xreal.client.ui.Component;
import xreal.client.ui.HorizontalAlignment;
import xreal.client.ui.Image;
import xreal.client.ui.Label;
import xreal.client.ui.StackPanel;
import xreal.client.ui.UserInterface;
import xreal.client.ui.VerticalAlignment;
import xreal.client.ui.event.KeyEvent;


/**
 * @author Robert Beckebans
 */
public class QuitMenu extends MenuFrame
{
	private Label						label;
	private Label						question;
	private StackPanel					stackPanel;
	private Button						yesButton;
	private Button						noButton;
	
	public QuitMenu()
	{
		super("menuback");
		
		backgroundImage.color.set(0, 0, 0, 0.5f);
		
		Map<String, String> navButtons = new HashMap<String, String>();
		navButtons.put("SELECT", "ui/keyboard_keys/standard_104/enter.png");
		navigationBar.set(navButtons);
		
		fullscreen = true;
		wrapAround = true;
		showlogo = true;
		
		Color backgroundColor = new Color(0.0f, 0.0f, 0.0f, 0.5f);
		
		label = new Label("WARNING");
		//label.height = 32;
		//label.margin.bottom = 26;
		label.width = UserInterface.SCREEN_WIDTH * 0.8f;
		label.textBlock.horizontalAlignment = HorizontalAlignment.Left;
		label.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		label.textBlock.fontSize = 34;
		label.textBlock.color.set(Color.LtGrey);
		label.backgroundImage = new Image("white");
		label.backgroundImage.color.set(backgroundColor);
		
		question = new Label("Are you sure you wish to quit?");
		question.margin.bottom = 10;
		question.width = UserInterface.SCREEN_WIDTH * 0.8f;
		question.textBlock.horizontalAlignment = HorizontalAlignment.Left;
		question.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		//question.textBlock.fontSize = 34;
		question.textBlock.color.set(Color.Yellow);
		question.backgroundImage = new Image("white");
		question.backgroundImage.color.set(backgroundColor);
		
		yesButton = new MenuButton("YES")
		{
			public void keyPressed(KeyEvent e)
			{
				KeyCode key = e.getKey();
				
				if(!e.isDown())
					return;
				
				Engine.println("yesButton.keyPressed(event = " + e + ")");
				
				switch(key)
				{
					case ENTER:
					case MOUSE1:
					case XBOX360_A:
						Engine.sendConsoleCommand(Engine.EXEC_APPEND, "quit");
						e.consume();
						break;
				}
			}
		};
		
		yesButton.height = 28;
		yesButton.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		yesButton.textBlock.fontSize = 22;
		yesButton.textBlock.color.set(Color.LtGrey);
		yesButton.backgroundImage = new Image("white");
		yesButton.backgroundImage.color.set(backgroundColor);
		
		noButton = new MenuButton("NO")
		{
			public void keyPressed(KeyEvent e)
			{
				KeyCode key = e.getKey();
				
				if(!e.isDown())
					return;
				
				Engine.println("noButton.keyPressed(event = " + e + ")");
				
				switch(key)
				{
					case ENTER:
					case MOUSE1:
					case XBOX360_A:
						UserInterface.popMenu();
						e.consume();
						break;
				}
			}
		};
		
		noButton.height = 28;
		noButton.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		noButton.textBlock.fontSize = 22;
		noButton.textBlock.color.set(Color.LtGrey);
		noButton.backgroundImage = new Image("white");
		noButton.backgroundImage.color.set(backgroundColor);
		
		
		
		
		stackPanel = new StackPanel();
		stackPanel.horizontalAlignment = HorizontalAlignment.Center;
		stackPanel.verticalAlignment = VerticalAlignment.Center;
		//stackPanel.margin.bottom = 100;
		//stackPanel.margin.left = UserInterface.SCREEN_WIDTH * 0.25f;
		
		stackPanel.addChild(label);
		stackPanel.addChild(question);
		stackPanel.addChild(yesButton);
		stackPanel.addChild(noButton);
		
		addChild(stackPanel);
		
		
		Vector<Component> order = new Vector<Component>();
		order.add(yesButton);
		order.add(noButton);
		setCursorOrder(order);
		
		setCursor(yesButton);
	}

}
