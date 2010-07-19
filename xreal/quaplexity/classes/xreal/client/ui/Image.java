package xreal.client.ui;

import xreal.CVars;
import xreal.client.renderer.Renderer;

/**
 * @author Robert Beckebans
 */
public class Image extends Component {

	int hMaterial;
	
	/**
	 * 
	 * @param materialName
	 */
	public Image(String materialName)
	{
		hMaterial = Renderer.registerMaterial(materialName);
	}
	
	public void render()
	{
		super.render();
		
		Rectangle rect = new Rectangle(bounds);
		UserInterface.adjustFrom640(rect);
		
		Renderer.drawStretchPic(rect.x, rect.y, rect.width, rect.height, 0, 0, 1, 1, hMaterial);
	}
}
