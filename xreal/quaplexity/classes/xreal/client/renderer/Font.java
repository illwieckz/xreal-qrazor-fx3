package xreal.client.renderer;

import xreal.CVars;
import xreal.Color;
import xreal.client.ui.Rectangle;
import xreal.client.ui.UserInterface;
import xreal.client.ui.border.LineBorder;

/**
 * @author Robert Beckebans
 */
public class Font
{

	public static final int		LEFT			= 0x00000000;					// default
	public static final int		CENTER			= 0x00000001;
	public static final int		RIGHT			= 0x00000002;
	public static final int		FORMATMASK		= 0x00000007;
	public static final int		DROPSHADOW		= 0x00000800;
	public static final int		BLINK			= 0x00001000;
	public static final int		INVERSE			= 0x00002000;
	public static final int		PULSE			= 0x00004000;
	public static final int		BOLD			= 0x00008000;

	private static final int	BLINK_DIVISOR	= 200;
	private static final float	PULSE_DIVISOR	= 75.0f;

	public static final int		GLYPH_START		= 0;
	public static final int		GLYPH_END		= 255;
	public static final int		GLYPH_CHARSTART	= 32;
	public static final int		GLYPH_CHAREND	= 127;
	public static final int		GLYPHS_PER_FONT	= GLYPH_END - GLYPH_START + 1;

	private static final char	Q_COLOR_ESCAPE	= '^';

	private Glyph				glyphs[];
	private float				glyphScale;
	private String				name;

	/**
	 * Usually only called by the engine.
	 * 
	 * @param glyphs
	 * @param glyphScale
	 * @param name
	 */
	public Font(Glyph[] glyphs, float glyphScale, String name)
	{
		super();
		this.glyphs = glyphs;
		this.glyphScale = glyphScale;
		this.name = name;

		// for debugging
		// Engine.println("Font(glyphScale = " + glyphScale + ", name = " + name);
		// for(int i = 0; i < GLYPHS_PER_FONT; i++) {
		// Engine.println("glyph " + i + "\n: " + glyphs[i].toString());
		// }

	}

	public void paintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, int hShader)
	{

		// Engine.println("Font.paintChar(x = " + x + ", y = " + y + ", width = " + width + ", height = " + height +
		// ", scale = " + scale + ", s = " + s + ", t = " + t + ", s2 = " + s2 + ", t2 = " + t2 + ", hShader = " +
		// hShader);

		Rectangle rect = new Rectangle(x, y, width * scale, height * scale);
		UserInterface.adjustFrom640(rect);

		Renderer.drawStretchPic(rect.x, rect.y, rect.width, rect.height, s, t, s2, t2, hShader);

		if(CVars.ui_debug.getBoolean())
		{
			LineBorder border = new LineBorder(Color.Red);
			border.paintBorder(x, y, width * scale, height * scale);
		}
	}

	public void paintText(float x, float y, float fontSize, Color color, String text, float adjust, int limit, int style)
	{
		int len;
		Color newColor = new Color(1, 1, 1, 1);
		Glyph glyph;
		float useScale;
		Color drawColor = new Color(1, 1, 1, 1);

		int textWidth = (int) getTextWidth(text, fontSize, 0);
		int textHeight = (int) getTextHeight(text, fontSize, 0);

		if((style & BLINK) != 0 && ((UserInterface.getRealTime() / BLINK_DIVISOR) & 1) != 0)
			return;

		switch(style & FORMATMASK)
		{
			case CENTER:
				x -= textWidth / 2.0f;
				break;

			case RIGHT:
				x -= textWidth;
				break;

			case LEFT:
			default:
				break;
		}

		if(CVars.ui_debug.getBoolean())
		{
			LineBorder border = new LineBorder(Color.Blue);
			border.paintBorder(x, y, textWidth, textHeight);
		}

		y += textHeight;// / 2.0f;

		drawColor.set(color);

		if((style & INVERSE) != 0)
		{
			drawColor.red = color.red * 0.8f;
			drawColor.green = color.green * 0.8f;
			drawColor.blue = color.blue * 0.8f;
			drawColor.alpha = color.alpha;
		}

		if((style & PULSE) != 0)
		{
			drawColor.alpha = (float) (0.7 + 0.3 * Math.sin(UserInterface.getRealTime() / PULSE_DIVISOR));
		}

		useScale = (fontSize / 48.0f) * glyphScale;
		if(text != null)
		{
			Renderer.setColor(drawColor);
			newColor.set(drawColor);

			len = text.length();
			if(limit > 0 && len > limit)
			{
				len = limit;
			}

			for(int i = 0; i < len;)
			{
				char ch = text.charAt(i);
				int chNumber = (int) ch;

				// Engine.println("Font.paintText(ch = " + ch + ", chNumber = " + chNumber);

				if(((len - i) >= 2) && (ch == Q_COLOR_ESCAPE) && (text.charAt(i + 1) != Q_COLOR_ESCAPE))// &&
																										// (Character.isDigit(text.charAt(i
																										// + 1 ))))
				{
					char colorChar = text.charAt(i + 1);

					switch(colorChar)
					{
						case '0':
							newColor.setRGB(Color.Black);
							break;

						case '1':
							newColor.setRGB(Color.Red);
							break;

						case '2':
							newColor.setRGB(Color.Green);
							break;

						case '3':
							newColor.setRGB(Color.Yellow);
							break;

						case '4':
							newColor.setRGB(Color.Blue);
							break;

						case '5':
							newColor.setRGB(Color.Cyan);
							break;

						case '6':
							newColor.setRGB(Color.Magenta);
							break;

						default:
						case '7':
							newColor.setRGB(Color.White);
							break;
					}

					Renderer.setColor(newColor);
					i += 2;
					continue;
				}
				else
				{
					if(chNumber < 32 || chNumber > 126)
						chNumber = 32;

					glyph = glyphs[chNumber];

					float yadj = useScale * glyph.top; // * (glyph.top / 2);

					if((style & DROPSHADOW) != 0) // || style == ITEM_TEXTSTYLE_SHADOWEDMORE)
					{
						int ofs = 1; // style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;

						Color shadowColor = new Color(Color.Black);
						shadowColor.alpha = newColor.alpha;

						Renderer.setColor(shadowColor);

						paintChar(x + ofs, y - yadj + ofs, glyph.imageWidth, glyph.imageHeight, useScale, glyph.s, glyph.t, glyph.s2, glyph.t2, glyph.glyph);

						Renderer.setColor(newColor);
					}

					paintChar(x, y - yadj, glyph.imageWidth, glyph.imageHeight, useScale, glyph.s, glyph.t, glyph.s2, glyph.t2, glyph.glyph);

					x += (glyph.xSkip * useScale) + adjust;
					i++;
				}
			}

			Renderer.setColor(Color.White);
		}
	}

	public String getName()
	{
		return name;
	}

	public float getTextWidth(String text, float fontSize, int limit)
	{
		int len;
		float out;
		Glyph glyph;
		float useScale;

		useScale = (fontSize / 48.0f) * glyphScale;
		out = 0;
		if(text != null)
		{
			len = text.length();
			if(limit > 0 && len > limit)
			{
				len = limit;
			}

			for(int i = 0; i < len;)
			{

				// check if a color string begins
				if(((len - i) > 2 && text.charAt(i) == Q_COLOR_ESCAPE) && (text.charAt(i + 1) != Q_COLOR_ESCAPE))
				{
					i += 2;
					continue;
				}
				else
				{
					char ch = text.charAt(i);
					int chNumber = (int) ch;

					if(chNumber < 32 || chNumber > 126)
						chNumber = 32;

					glyph = glyphs[chNumber];
					out += glyph.xSkip;
					i++;
				}
			}
		}

		return out * useScale;
	}

	public float getTextHeight(String text, float fontSize, int limit)
	{
		int len;
		float max;
		Glyph glyph;
		float useScale;

		useScale = (fontSize / 48.0f) * glyphScale;
		max = 0;
		if(text != null)
		{
			len = text.length();
			if(limit > 0 && len > limit)
			{
				len = limit;
			}

			for(int i = 0; i < len;)
			{

				// check if a color string begins
				if(((len - i) > 2 && text.charAt(i) == Q_COLOR_ESCAPE) && (text.charAt(i + 1) != Q_COLOR_ESCAPE))
				{
					i += 2;
					continue;
				}
				else
				{
					char ch = text.charAt(i);
					int chNumber = (int) ch;

					if(chNumber < 0 || chNumber > GLYPH_END)
						chNumber = 32;

					glyph = glyphs[chNumber];
					if(max < glyph.height)
					{
						max = glyph.height;
					}
					i++;
				}
			}
		}

		return max * useScale;
	}

	public Rectangle getTextBounds(String text, float fontSize, int limit)
	{

		float w = getTextWidth(text, fontSize, limit);
		float h = getTextHeight(text, fontSize, limit);

		Rectangle rect = new Rectangle(0, 0, w, h);

		return rect;
	}
}
