package xreal.client.game;

import com.bulletphysics.BulletStats;

import xreal.Color;
import xreal.client.renderer.Font;
import xreal.client.renderer.Renderer;
import xreal.client.ui.Component;
import xreal.client.ui.Image;
import xreal.client.ui.UserInterface;

public class BulletStatsView extends Component
{
	private int		whiteMaterial;
	private Font	font;
	private Image	backgroundImage;
	
	public BulletStatsView()
	{
		// place it into the lower right corner
		setX(20);
		setY(20);
		setWidth(200);
		setHeight(200);
		
		whiteMaterial = Renderer.registerMaterialNoMip("white");
		font = Renderer.registerFont("fonts/Vera.ttf", 48);
		
		backgroundImage = new Image("lagometer2");
		backgroundImage.setBounds(this.getBounds());
	}
	
	@Override
	public void render()
	{
		// TODO
		String s = "gNumDeepPenetrationChecks = " + BulletStats.gNumDeepPenetrationChecks;

		font.paintText(getX(), getY(), 10, Color.White, s, 0, 0, Font.RIGHT | Font.DROPSHADOW);
	}
}
