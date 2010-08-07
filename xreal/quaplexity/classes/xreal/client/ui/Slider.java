package xreal.client.ui;


/**
 * @author Robert Beckebans
 */
public class Slider extends Component
{
	//public Image		bar;
	public Image		barImage;
	public Image		thumb;
	
	public Slider()
	{
		super();
		
		//orientation = Orientation.Horizontal;
		
		width = 100;
		height = 26;
		
		
		backgroundImage = new Image("white");
		backgroundImage.color.set(0, 0, 0, 0.5f);
		
		barImage = new Image("ui/slider.png");
		barImage.horizontalAlignment = HorizontalAlignment.Stretch;
		barImage.verticalAlignment = VerticalAlignment.Stretch;
		barImage.margin.left = 0;
		barImage.margin.top = 5;
		barImage.margin.right = 0;
		barImage.margin.bottom = 5;
		
		thumb = new Image("ui/sliderbutt_1");
		
		addChild(barImage);
		//addChild(thumb);
	}
}
