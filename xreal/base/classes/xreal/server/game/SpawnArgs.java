package xreal.server.game;

import javax.vecmath.Point3f;
import javax.vecmath.Vector3f;

import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import xreal.Angle3f;

public class SpawnArgs {
	
	Element	root;
	
	/**
	 * @param root
	 * 				Element with the <entity> tag.
	 */
	public SpawnArgs(Element root) {
		this.root = root;
	}
	
	public String getClassName() {
		
		String defaultValue = "unknown";
		try {
			String s = root.getAttribute("class");
			if (s == null)
				return s;
			else
				return s;
		} catch (Throwable t) {
			return defaultValue;
		}
	}
	
	/**
	 * Lookup a string spawn argument.
	 * 
	 * @return value found, or defaultValue.
	 * @param tagName
	 *            name of sub-element we're looking for.
	 * @param defaultValue
	 *            value to return if "tagName" is not found.
	 */
	public String getString(String tagName, String defaultValue) {
		try {
			Element e2 = (Element) (root.getElementsByTagName(tagName).item(0));
			return e2.getFirstChild().getNodeValue();
		} catch (Throwable t) {
			return defaultValue;
		}
	}

	/**
	 * Lookup a string spawn argument.
	 * 
	 * @return value found, or defaultValue.
	 * @param tagName
	 *            name of sub-element we're looking for.
	 * @param defaultValue
	 *            value to return if "tagName" is not found.
	 */
	public String getString(String tagName, String attributeName, String defaultValue) {
		try {
			Element e2 = (Element) (root.getElementsByTagName(tagName).item(0));
			String s = e2.getAttribute(attributeName);
			if (s == null)
				return defaultValue;
			else
				return s;
		} catch (Throwable t) {
			return defaultValue;
		}
	}
	
	/**
	 * Lookup a float spawn argument.
	 * 
	 * @return value found, or defaultValue.
	 * @param tagName
	 *            name of sub-element we're looking for.
	 * @param defaultValue
	 *            value to return if "tagName" is not found.
	 */
	public float getFloat(String tagName, float defaultValue) {
		try {
			Element e2 = (Element) (root.getElementsByTagName(tagName).item(0));
			return Float.valueOf(e2.getFirstChild().getNodeValue()).floatValue();
		} catch (Throwable t) {
			return defaultValue;
		}
	}

	/**
	 * Lookup an int spawn argument.
	 * 
	 * @return value found, or defaultValue.
	 * @param tagName
	 *            name of sub-element we're looking for.
	 * @param defaultValue
	 *            value to return if "tagName" is not found.
	 */
	public int getInt(String tagName, int defaultValue) {
		try {
			Element e2 = (Element) (root.getElementsByTagName(tagName).item(0));
			return Integer.parseInt(e2.getFirstChild().getNodeValue());
		} catch (Throwable t) {
			return defaultValue;
		}
	}
	
	/**
	 * Get a Vector3f from a given document element.
	 * 
	 * @return Origin of item, or null if it can't be determined or the info in
	 *         the document is invalid.
	 * @param e
	 *            org.w3c.dom.Element
	 */
	private Vector3f getVector3f(Element e) {
		try {
			Vector3f p = new Vector3f();

			String s = e.getAttribute("x");
			if (s != null)
				p.x = Float.valueOf(s).floatValue();

			s = e.getAttribute("y");
			if (s != null)
				p.y = Float.valueOf(s).floatValue();

			s = e.getAttribute("z");
			if (s != null)
				p.z = Float.valueOf(s).floatValue();

			return p;
		} catch (Throwable t) {
			// either there was no origin element, or one or more of the
			// x,y,z attributes was malformed
			return null;
		}
	}

	/**
	 * Get a Vector3f from a sub-element of a given document element.
	 * 
	 * @return Origin of item, or null if it can't be determined or the info in
	 *         the document is invalid.
	 * @param tagName
	 *            name of sub-element containing the point information.
	 */
	public Vector3f getVector3f(String tagName) {
		try {
			NodeList nl = root.getElementsByTagName(tagName);

			return getVector3f((Element) nl.item(0));
		} catch (Throwable t) {
			// probably no element with the given tagname
			return null;
		}
	}
	
	/**
	 * Get a Point3f from a given document element.
	 * 
	 * @return Origin of item, or null if it can't be determined or the info in
	 *         the document is invalid.
	 * @param e
	 *            org.w3c.dom.Element
	 */
	private Point3f getPoint3f(Element e) {
		try {
			Point3f p = new Point3f();

			String s = e.getAttribute("x");
			if (s != null)
				p.x = Float.valueOf(s).floatValue();

			s = e.getAttribute("y");
			if (s != null)
				p.y = Float.valueOf(s).floatValue();

			s = e.getAttribute("z");
			if (s != null)
				p.z = Float.valueOf(s).floatValue();

			return p;
		} catch (Throwable t) {
			// either there was no origin element, or one or more of the
			// x,y,z attributes was malformed
			return null;
		}
	}

	/**
	 * Get a Point3f from a sub-element of a given document element.
	 * 
	 * @return Origin of item, or null if it can't be determined or the info in
	 *         the document is invalid.
	 * @param tagName
	 *            name of sub-element containing the point information.
	 */
	public Point3f getPoint3f(String tagName) {
		try {
			NodeList nl = root.getElementsByTagName(tagName);

			return getPoint3f((Element) nl.item(0));
		} catch (Throwable t) {
			// probably no element with the given tagname
			return null;
		}
	}
	
	/**
	 * Get an Angle3f from a given document element.
	 * 
	 * @return Origin of item, or null if it can't be determined or the info in
	 *         the document is invalid.
	 * @param e
	 *            org.w3c.dom.Element
	 */
	private Angle3f getAngle3f(Element e) {
		try {
			Angle3f p = new Angle3f();

			String s = e.getAttribute("pitch");
			if (s != null)
				p.x = Float.valueOf(s).floatValue();

			s = e.getAttribute("yaw");
			if (s != null)
				p.y = Float.valueOf(s).floatValue();

			s = e.getAttribute("roll");
			if (s != null)
				p.z = Float.valueOf(s).floatValue();

			return p;
		} catch (Throwable t) {
			// probably one or more of the
			// pitch,yaw,roll attributes was malformed
			return null;
		}
	}

	/**
	 * Get an Angle3f from a sub-element of a given document element.
	 * 
	 * @return Origin of item, or null if it can't be determined or the info in
	 *         the document is invalid.
	 * @param e
	 *            org.w3c.dom.Element
	 * @param tagName
	 *            name of sub-element containing the point information.
	 */
	public Angle3f getAngle3f(String tagName) {
		try {
			NodeList nl = root.getElementsByTagName(tagName);
			return getAngle3f((Element) nl.item(0));
		} catch (Throwable t) {
			// probably no element with the given tagname
			return null;
		}
	}

	

	/**
	 * Get the spawn flags for a map element.
	 * 
	 * @return value found, or 0 if not found.
	 * @param e
	 *            DOM element describing a particular map entity.
	 */
	/*
	public static int getSpawnFlags(Element e) {
		try {
			return Integer.parseInt(e.getAttribute("spawnflags"));
		} catch (Throwable t) {
			return 0;
		}
	}
	*/
	

	
}
