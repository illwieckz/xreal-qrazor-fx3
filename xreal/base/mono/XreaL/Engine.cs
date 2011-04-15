using System;
using System.Runtime.CompilerServices;
namespace XreaL
{
	/// <summary>
	/// Main interface to the XreaL engine.
	/// </summary>
	public class Engine
	{
		/// <summary>
		/// Absolute limit. Don't change this here unless you increase it in q_shared.h 
		/// </summary>
		public const int MAX_CLIENTS = 64; // absolute limit
		
		/// <summary>
		/// Absolute limit. Don't change this here unless you increase it in q_shared.h 
		/// </summary>
		public const int MAX_LOCATIONS = 64;
		
		/// <summary>
		/// Absolute limit. Don't change this here unless you increase it in q_shared.h
		/// The Engine won't send more bits for entityState_t::index
		/// </summary>
		private const int GENTITYNUM_BITS = 11;
		public const int MAX_GENTITIES = (1 << GENTITYNUM_BITS);
	
		/// <summary>
		/// Entitynums are communicated with GENTITY_BITS, so any reserved
		/// values that are going to be communicated over the net need to
		/// also be in this range.
		/// </summary>
		public const int ENTITYNUM_NONE = (MAX_GENTITIES - 1);
		public const int ENTITYNUM_WORLD = (MAX_GENTITIES - 2);
		public const int ENTITYNUM_MAX_NORMAL = (MAX_GENTITIES - 2);
	
		// Tr3B: if you increase GMODELNUM_BITS then:
		// increase MAX_CONFIGSTRINGS to 2048 and double MAX_MSGLEN
		public const int GMODELNUM_BITS = 9; // don't need to send any more
		public const int MAX_MODELS = (1 << GMODELNUM_BITS); 		// references
																	// entityState_t::modelindex
	
		public const int MAX_SOUNDS = 256; // so they cannot be blindly
												// increased
		public const int MAX_EFFECTS = 256;
	
		public const int MAX_CONFIGSTRINGS = (1024 * 2);
		
		
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public static extern void Print(string s);
	
		public static void PrintLine(string s)
		{
			//Console.WriteLine(s);
			
			Print(s + "\n");
		}
	
		/// <summary>
		///  Tell the engine that a really bad error happened and quit the game but not the engine.
		///	
		///  This is equivalent to Com_Error(ERR_DROP, "%s", s)
		/// </summary>
		//public void Error(string s);
		
		/// <summary>
		/// Return the current time using Sys_Milliseconds(); 
		/// </summary>
		//public static int getTimeInMilliseconds();
		
		/// <summary>
		/// Cmd_Argv(0) to Cmd_Argv(Cmd_Argc()-1)
		/// </summary>
		/// <returns>
		/// A <see cref="Returns"/>
		/// </returns>
		//public static string[] getConsoleArgs();
		/*
		public static string concatConsoleArgs(int start) 
		{
			string line = "";
	
			string[] args = Engine.getConsoleArgs();
			
			if(args != null)
			{
				int c = args.length;
				for (int i = start; i < c; i++) {
					string arg = args[i];
		
					line += arg;
		
					if (i != c - 1) {
						line += ' ';
					}
				}
			}
	
			return line;
		}
		*/
	
		
		/**
		 *  Don't return until completed, a VM should NEVER use this,
		 *  because some commands might cause the VM to be unloaded...
		 */
		public const int EXEC_NOW = 0;
		
		/**
		 * Insert at current position, but don't run yet.
		 */
		public const int EXEC_INSERT = 1; 
		
		/**
		 * Add to end of the command buffer. (normal case)
		 */
		public const int EXEC_APPEND = 2;
		
		
		/**
		 * Send a console command to the Cmd_ module in the engine.
		 * 
		 * @param exec_when One of the EXEC_* flags.
		 * @param text		The console command.
		 */
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public static extern void sendConsoleCommand(int exec_when, string text);
		
		/**
		 * Read an entire file into an array of bytes at once.
		 * 
		 * @param filename The filename is relative to the quake search path.
		 * 					e.g.:
		 * 					models/mymodel/yo.md5mesh
		 * 					maps/mymap.bsp
		 * 
		 * @return The byte array.
		 */
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public static extern byte[] readFile(string fileName);
		
		/**
		 * Write an entire file as an array of bytes at once.
		 * 
		 * @param filename The filename is relative to the quake search path.
		 * 					e.g.:
		 * 					models/mymodel/yo.md5mesh
		 * 					maps/mymap.bsp
		 */
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		public static extern void writeFile(string fileName, byte[] data);
	}
}

