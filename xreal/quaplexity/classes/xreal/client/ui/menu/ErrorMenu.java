package xreal.client.ui.menu;

import java.util.Vector;

import xreal.CVars;
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
import xreal.client.ui.event.KeyListener;

/**
 * @author Robert Beckebans
 */
public class ErrorMenu extends MenuFrame
{
	private Label						title;
	private Label						message;
	private Label						stackTraceHeader;
	private Label						stackTrace;
	private StackPanel					stackPanel;
	
	public ErrorMenu()
	{
		super("menuback");
		
		fullscreen = false;
		
		backgroundImage.color.set(0, 0, 0, 0.5f);
		
		Color backgroundColor = new Color(0.0f, 0.0f, 0.0f, 0.5f);
		
		title = new Label("ERROR");
		//title.height = 32;
		//title.margin.bottom = 26;
		title.width = UserInterface.SCREEN_WIDTH * 0.8f;
		title.textBlock.horizontalAlignment = HorizontalAlignment.Left;
		title.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		title.textBlock.fontSize = 34;
		title.textBlock.color.set(Color.LtGrey);
		title.backgroundImage = new Image("white");
		title.backgroundImage.color.set(backgroundColor);
		
		message = new Label();
		message.margin.bottom = 10;
		message.width = UserInterface.SCREEN_WIDTH * 0.8f;
		message.textBlock.horizontalAlignment = HorizontalAlignment.Left;
		message.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		//message.textBlock.fontSize = 34;
		message.textBlock.color.set(Color.Red);
		message.backgroundImage = new Image("white");
		message.backgroundImage.color.set(backgroundColor);
		
		stackTraceHeader = new Label("STACKTRACE");
		stackTraceHeader.margin.top = 10;
		stackTraceHeader.width = UserInterface.SCREEN_WIDTH * 0.8f;
		stackTraceHeader.textBlock.horizontalAlignment = HorizontalAlignment.Left;
		stackTraceHeader.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		//stackTraceHeader.textBlock.fontSize = 34;
		stackTraceHeader.textBlock.color.set(Color.LtGrey);
		stackTraceHeader.backgroundImage = new Image("white");
		stackTraceHeader.backgroundImage.color.set(backgroundColor);
		
		stackTrace = new Label();
		//stackTrace.margin.top = 10;
		stackTrace.width = UserInterface.SCREEN_WIDTH * 0.8f;
		stackTrace.textBlock.horizontalAlignment = HorizontalAlignment.Left;
		stackTrace.textBlock.font = Renderer.registerFont("fonts/FreeSansBold.ttf", 48);
		//stackTrace.textBlock.fontSize = 34;
		stackTrace.textBlock.color.set(Color.White);
		stackTrace.backgroundImage = new Image("white");
		stackTrace.backgroundImage.color.set(backgroundColor);
		
		
		
		
		stackPanel = new StackPanel();
		stackPanel.horizontalAlignment = HorizontalAlignment.Center;
		stackPanel.verticalAlignment = VerticalAlignment.Center;
		//stackPanel.margin.bottom = 100;
		//stackPanel.margin.left = UserInterface.SCREEN_WIDTH * 0.25f;
		
		stackPanel.addChild(title);
		stackPanel.addChild(message);
		stackPanel.addChild(stackTraceHeader);
		stackPanel.addChild(stackTrace);
		
		addChild(stackPanel);
		
		
		Vector<Component> order = new Vector<Component>();
		setCursorOrder(order);
		//setCursor(yesButton);
	}
	
	public void setErrorMessage(String msg, String stackTrace)
	{
		message.textBlock.text = msg;
		
		
		this.stackTrace.textBlock.text = stackTrace;
	}

	@Override
	protected void updateNavigationBarPC()
	{
		navigationBar.clear();
		navigationBar.add("RETURN", "ui/keyboard_keys/standard_104/enter.png");
		
		super.updateNavigationBarPC();
	}
	
	@Override
	protected void updateNavigationBar360()
	{
		navigationBar.clear();
		navigationBar.add("RETURN", "ui/xbox360/xna/buttons/xboxControllerButtonA.png");
		
		super.updateNavigationBar360();
	}
	
	@Override
	public void keyPressed(KeyEvent e)
	{
		KeyCode key = e.getKey();
		
		switch(key)
		{
			case ESCAPE:
			case ENTER:
			case MOUSE1:
			case XBOX360_A:
				CVars.com_errorMessage.set("");
				CVars.com_stackTrace.set("");
				UserInterface.popMenu();
				break;
				
			case F12:
				Engine.sendConsoleCommand(Engine.EXEC_APPEND, "screenshotJPEG\n");
				break;
		}
	}
}
