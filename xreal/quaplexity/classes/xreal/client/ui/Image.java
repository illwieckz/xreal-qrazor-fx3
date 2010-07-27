package xreal.client.ui;

import xreal.CVars;
import xreal.Color;
import xreal.client.renderer.Renderer;

/**
 * @author Robert Beckebans
 */
public class Image extends Component
{
	private int	hMaterial;

	public Image(String materialName)
	{
		hMaterial = Renderer.registerMaterial(materialName);
	}

	public Image(String materialName, Color color)
	{
		hMaterial = Renderer.registerMaterial(materialName);
		this.color = color;
	}

	public void render()
	{
		super.render();
		
		Renderer.setColor(color);

		Rectangle rect;
		try
		{
			rect = new Rectangle(getBounds());
			UserInterface.adjustFrom640(rect);

			Renderer.drawStretchPic(rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, hMaterial);
		}
		catch(Exception e)
		{
			// TODO Auto-generated catch block
			//e.printStackTrace();
		}
		finally
		{
			Renderer.setColor(Color.White);
		}
	}
}
