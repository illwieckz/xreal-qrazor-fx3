package xreal;

public class Engine {
	public synchronized native static void print(String s);

	public synchronized static void println(String s) {
		print(s + "\n");
	}

	public synchronized native static void error(String s);

	public synchronized native static int milliseconds();

	public synchronized native static int consoleArgc();

	public synchronized native static String consoleArgv(int n);

	public synchronized native static String consoleArgs();

	// public native static void sendConsoleCommand(int exec_when, String text);
	// void trap_DropClient(int clientNum, const char *reason);
	// void trap_SendServerCommand(int clientNum, const char *text);
	public synchronized native static String getConfigstring(int num);

	public synchronized native static void setConfigstring(int num,
			String string);

	public synchronized native static String getUserinfo(int num);

	public synchronized native static String setUserinfo(int num, String buffer);

	public synchronized native static String getServerinfo();
}
