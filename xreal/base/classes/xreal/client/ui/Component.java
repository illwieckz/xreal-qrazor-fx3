package xreal.client.ui;

import xreal.Engine;
import xreal.client.ui.border.Border;
import xreal.client.ui.event.Event;
import xreal.client.ui.event.EventListener;

/**
 * 
 * @author Robert Beckebans
 */
public class Component implements EventListener {

	public static final int QMF_BLINK				= 0x00000001;
	public static final int QMF_SMALLFONT			= 0x00000002;
	public static final int QMF_LEFT_JUSTIFY		= 0x00000004;
	public static final int QMF_CENTER_JUSTIFY		= 0x00000008;
	public static final int QMF_RIGHT_JUSTIFY		= 0x00000010;
	public static final int QMF_NUMBERSONLY			= 0x00000020;	// edit field is only numbers
	public static final int QMF_HIGHLIGHT			= 0x00000040;
	public static final int QMF_HIGHLIGHT_IF_FOCUS	= 0x00000080;	// steady focus
	public static final int QMF_PULSEIFFOCUS		= 0x00000100;	// pulse if focus
	public static final int QMF_HASMOUSEFOCUS		= 0x00000200;
	public static final int QMF_NOONOFFTEXT			= 0x00000400;
	public static final int QMF_MOUSEONLY			= 0x00000800;	// only mouse input allowed
	public static final int QMF_HIDDEN				= 0x00001000;	// skips drawing
	public static final int QMF_GRAYED				= 0x00002000;	// grays and disables
	public static final int QMF_INACTIVE			= 0x00004000;	// disables any input
	public static final int QMF_NODEFAULTINIT		= 0x00008000;	// skip default initialization
	public static final int QMF_OWNERDRAW			= 0x00010000;
	public static final int QMF_PULSE				= 0x00020000;
	public static final int QMF_LOWERCASE			= 0x00040000;	// edit field is all lower case
	public static final int QMF_UPPERCASE			= 0x00080000;	// edit field is all upper case
	public static final int QMF_SILENT				= 0x00100000;
	
	public int flags;
	
	protected Rectangle bounds = new Rectangle(0, 0, 0, 0);
	protected Border border;
	
	@Override
	public void processEvent(Event e) {
	}
	
	public Rectangle getBounds() {
		return bounds;
	}
	
	public void setLocation(float x, float y) {
		bounds.setLocation(x, y);
	}
	
	public void setCenter(float x, float y) {
		bounds.setCenter(x, y);
	}
	
	public void setXCenter(float x) {
		bounds.setXCenter(x);
	}
	
	public void setYCenter(float y) {
		bounds.setYCenter(y);
	}
	
	public boolean contains(float x, float y) {
		return bounds.contains(x, y);
	}

	public void render() {
	}
	
	public float getX() {
		return bounds.x;
	}
	
	public float getY() {
		return bounds.y;
	}
	
	public float getWidth() {
		return bounds.width;
	}
	
	public float getHeight() {
		return bounds.height;
	}
	
	public void setX(float f) {
		bounds.x = f;
	}
	
	public void setY(float f) {
		bounds.y = f;
	}
	
	public void setWidth(float f) {
		bounds.width = f;
	}
	
	public void setHeight(float f) {
		bounds.height = f;
	}

	public void setBorder(Border border) {
		this.border = border;
	}

	public Border getBorder() {
		return border;
	}
	
	public void addFlags(int flags) {
		this.flags |= flags;
	}
	
	public void delFlags(int flags) {
		this.flags &= ~flags;
	}
	
	public boolean hasFlags(int flags) {
		return (this.flags & flags) != 0;
	}
}
