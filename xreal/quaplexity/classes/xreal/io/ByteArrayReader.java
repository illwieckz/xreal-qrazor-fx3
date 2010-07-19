package xreal.io;

import java.io.ByteArrayInputStream;
import java.io.IOException;

import xreal.Engine;


/**
 * ByteArrayReader is designed to work with byte[] Engine.readFile().
 * 
 * @author Robert Beckebans
 */
public class ByteArrayReader extends ByteArrayInputStream {

	public ByteArrayReader(byte[] buf) {
		super(buf);
	}
	
	public ByteArrayReader(byte[] buf, int offset, int length) {
		super(buf, offset, length);
	}
	
	public short readShort() throws IOException {
		int[] b = new int[2];
		
		b[0] = read();
		b[1] = read();
		
		return (short) ((b[1] << 8) + b[0]);
	}

	public int readInt() throws IOException {
		int[] b = new int[4];
		
		b[0] = read();
		b[1] = read();
		b[2] = read();
		b[3] = read();
		
		//Engine.println("b = [" + b[0] + ","+ b[1] + ","+ b[2] + ","+ b[3] + "]");
		
		return (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + b[0];
	}
	
	public float readFloat() throws IOException {
		int i = readInt();

		return Float.intBitsToFloat(i);
	}
}
