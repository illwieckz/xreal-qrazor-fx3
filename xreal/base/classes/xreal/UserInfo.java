package xreal;

import java.util.Enumeration;
import java.util.Hashtable;
import java.util.StringTokenizer;

/**
 * UserInfo represents a Q3A user info string which is a set of (key, value) pairs.
 * 
 * @author Robert Beckebans
 */
public class UserInfo {
	private Hashtable<String, String> table = new Hashtable<String, String>();

	/**
	 * Convert the userinfo string into the HashTable.
	 * 
	 * @param userinfo
	 *            the userinfo string, formatted as:
	 *            "\keyword\value\keyword\value\....\keyword\value"
	 */
	public void read(String userinfo) {
		StringTokenizer st = new StringTokenizer(userinfo, "\\");
		while (st.hasMoreTokens()) {
			String key = st.nextToken();
			if (st.hasMoreTokens()) {
				String val = st.nextToken();
				String oldVal = (String) table.get(key);

				if ((oldVal == null) || (!val.equals(oldVal))) {
					table.put(key, val);
				}
			}
		}
	}

	/**
	 * Get userinfo value by key.
	 * 
	 * @param key
	 * 
	 * @return The value.
	 */
	public String get(String key) {
		return table.get(key);
	}

	/**
	 * Changes or adds a key/value pair
	 * 
	 * @param key
	 * @param value
	 */
	public void put(String key, String value) {
		final char blacklist[] = { '\\', ';', '\"' };

		for (int i = 0; i < blacklist.length; i++) {
			char c = blacklist[i];

			if (value.indexOf(c) != -1) {
				Engine.print(ConsoleColorStrings.YELLOW + "Can't use keys or values with a '" + c + "': " + key + " = " + value);
				return;
			}
		}

		table.put(key, value);
	}

	public void put(String key, float value) {
		put(key, Float.toString(value));
	}

	public void put(String key, int value) {
		put(key, Integer.toString(value));
	}
	
	public void put(String key, boolean value) {
		put(key, Boolean.toString(value));
	}

	/**
	 * Convert the internal representation to Q3A's userinfo string format
	 * "\keyword\value\keyword\value\....\keyword\value".
	 */
	public String toString() {
		String r = "";
		Enumeration<String> keys = table.keys();

		while (keys.hasMoreElements()) {
			String key = keys.nextElement();
			String value = table.get(key);

			r += "\\" + key + "\\" + value;
		}

		return r;
	}
}
