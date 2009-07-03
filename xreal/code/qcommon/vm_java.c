/*
===========================================================================
Copyright (C) 2009 Robert Beckebans <trebor_7@users.sourceforge.net>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#if defined(USE_JAVA)

#ifdef WIN32
#define DEFAULT_JAVA_LIB "jvm.dll"
#elif defined(MACOS_X)
#define DEFAULT_JAVA_LIB "java.dylib"
#else
#define DEFAULT_JAVA_LIB "/usr/lib/jvm/java-6-openjdk/jre/lib/i386/server/libjvm.so"	//"java.so" //
#endif

#include "q_shared.h"
#include "qcommon.h"
#include "vm_java.h"
#include "../sys/sys_loadlib.h"

#include "java/xreal_Engine.h"
#include "java/xreal_CVar.h"

static cvar_t  *jvm_javaLib;

JNIEnv         *javaEnv;
JavaVM         *javaVM;
static void    *javaLib = NULL;
qboolean        javaEnabled = qfalse;
qboolean        javaVMIsOurs = qfalse;

jint(*QJNI_CreateJavaVM) (JavaVM ** p_vm, JNIEnv ** p_env, void *vm_args);
jint(*QJNI_GetCreatedJavaVMs) (JavaVM ** vmBuf, jsize bufLen, jsize * nVMs);



static void    *GPA(char *str)
{
	void           *rv;

	rv = Sys_LoadFunction(javaLib, str);
	if(!rv)
	{
		Com_Printf("Can't load symbol %s\n", str);
		javaEnabled = qfalse;
		return NULL;
	}
	else
	{
		Com_DPrintf("Loaded symbol %s (0x%p)\n", str, rv);
		return rv;
	}
}


static void JVM_JNI_Shutdown(void)
{
	if(javaLib)
	{
		Sys_UnloadLibrary(javaLib);
		javaLib = NULL;
	}

	QJNI_CreateJavaVM = NULL;
	QJNI_GetCreatedJavaVMs = NULL;
}

static qboolean JVM_JNI_Init()
{
	if(javaLib)
		return qtrue;

	Com_Printf("Loading \"%s\"...\n", jvm_javaLib->string);
	if((javaLib = Sys_LoadLibrary(jvm_javaLib->string)) == 0)
	{
#ifdef _WIN32
		return qfalse;
#else
		char            fn[1024];

		Com_Printf("JVM_JNI_Init() failed:\n\"%s\"\n", Sys_LibraryError());

		Q_strncpyz(fn, Sys_Cwd(), sizeof(fn));
		strncat(fn, "/", sizeof(fn) - strlen(fn) - 1);
		strncat(fn, jvm_javaLib->string, sizeof(fn) - strlen(fn) - 1);

		if((javaLib = Sys_LoadLibrary(fn)) == 0)
		{
			Com_Printf("JVM_JNI_Init() failed:\n\"%s\"\n", Sys_LibraryError());
			return qfalse;
		}
#endif							/* _WIN32 */
	}

	javaEnabled = qtrue;

	QJNI_CreateJavaVM = GPA("JNI_CreateJavaVM");
	QJNI_GetCreatedJavaVMs = GPA("JNI_GetCreatedJavaVMs");

	if(!javaEnabled)
	{
		//JVM_JNI_Shutdown();
		return qfalse;
	}

	return qtrue;
}

// ====================================================================================


// handles to java.lang.Throwable class
static jclass   class_Throwable;
static jmethodID method_Throwable_printStackTrace;
jmethodID method_Throwable_getMessage;

/**
 * @brief Convert a Java string (which is Unicode) to reasonable 7-bit ASCII.
 *
 * @author Berry Pederson
 */
void Misc_javaRegister()
{
	class_Throwable = (*javaEnv)->FindClass(javaEnv, "java/lang/Throwable");
	if(!class_Throwable)
	{
		Com_Error(ERR_FATAL, "Couldn't find java.lang.Throwable");
	}

	method_Throwable_printStackTrace = (*javaEnv)->GetMethodID(javaEnv, class_Throwable, "printStackTrace", "()V");
	if(!method_Throwable_printStackTrace)
	{
		Com_Error(ERR_FATAL, "Couldn't find java.lang.Throwable.printStackTrace() method");
	}

	method_Throwable_getMessage = (*javaEnv)->GetMethodID(javaEnv, class_Throwable, "getMessage", "()Ljava/lang/String;");
	if(!method_Throwable_getMessage)
	{
		Com_Error(ERR_FATAL, "Couldn't find java.lang.Throwable.getMessage() method");
	}

	// now that the java.lang.Class and java.lang.Throwable handles are obtained
	// we can start checking for exceptions
}

void Misc_javaDetach()
{
	(*javaEnv)->DeleteLocalRef(javaEnv, class_Throwable);
}


// ====================================================================================

/*
 * Class:     xreal_CVar
 * Method:    register0
 * Signature: (Ljava/lang/String;Ljava/lang/String;I)I
 */
jint JNICALL Java_xreal_CVar_register0(JNIEnv * env, jclass cls, jstring jname, jstring jvalue, jint flags)
{
	//Cvar_Register(VMA(1), VMA(2), VMA(3), args[4]);

	char           *varName;
	char           *defaultValue;
	cvar_t         *cv;

	varName = (char *)((*env)->GetStringUTFChars(env, jname, 0));
	defaultValue = (char *)((*env)->GetStringUTFChars(env, jvalue, 0));

	cv = Cvar_Get(varName, defaultValue, flags);

	(*env)->ReleaseStringUTFChars(env, jname, varName);
	(*env)->ReleaseStringUTFChars(env, jvalue, defaultValue);

	//CheckException();

	return (jint) cv;
}

/*
 * Class:     xreal_CVar
 * Method:    set0
 * Signature: (Ljava/lang/String;Ljava/lang/String;)V
 */
void JNICALL Java_xreal_CVar_set0(JNIEnv * env, jclass cls, jstring jname, jstring jvalue)
{
	// Cvar_Set((const char *)VMA(1), (const char *)VMA(2));

	char           *varName;
	char           *value;

	varName = (char *)((*env)->GetStringUTFChars(env, jname, 0));
	value = (char *)((*env)->GetStringUTFChars(env, jvalue, 0));

	Cvar_Set(varName, value);

	(*env)->ReleaseStringUTFChars(env, jname, varName);
	(*env)->ReleaseStringUTFChars(env, jvalue, value);

	//CheckException();
}

/*
 * Class:     xreal_CVar
 * Method:    getString0
 * Signature: (I)Ljava/lang/String;
 */
jstring JNICALL Java_xreal_CVar_getString0(JNIEnv * env, jclass cls, jint ptr)
{
	cvar_t         *cv = (cvar_t *) ptr;

	return (*env)->NewStringUTF(env, cv->string);
}

/*
 * Class:     xreal_CVar
 * Method:    getValue0
 * Signature: (I)F
 */
jfloat JNICALL Java_xreal_CVar_getValue0(JNIEnv * env, jclass cls, jint ptr)
{
	cvar_t         *cv = (cvar_t *) ptr;

	return cv->value;
}

/*
 * Class:     xreal_CVar
 * Method:    getInteger0
 * Signature: (I)I
 */
jint JNICALL Java_xreal_CVar_getInteger0(JNIEnv * env, jclass cls, jint ptr)
{
	cvar_t         *cv = (cvar_t *) ptr;

	return cv->integer;
}



// handle to CVar class
static jclass   class_CVar;
static JNINativeMethod CVar_methods[] = {
	{"register0", "(Ljava/lang/String;Ljava/lang/String;I)I", Java_xreal_CVar_register0},
	{"set0", "(Ljava/lang/String;Ljava/lang/String;)V", Java_xreal_CVar_set0},
	{"getString0", "(I)Ljava/lang/String;", Java_xreal_CVar_getString0},
	{"getValue0", "(I)F", Java_xreal_CVar_getValue0},
	{"getInteger0", "(I)I", Java_xreal_CVar_getInteger0},
};

void CVar_javaRegister()
{
	class_CVar = (*javaEnv)->FindClass(javaEnv, "xreal/CVar");
	if(CheckException() || !class_CVar)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.CVar");
	}

	(*javaEnv)->RegisterNatives(javaEnv, class_CVar, CVar_methods, sizeof(CVar_methods) / sizeof(CVar_methods[0]));
	if(CheckException())
	{
		Com_Error(ERR_FATAL, "Couldn't register native methods for xreal.CVar");
	}
}


void CVar_javaDetach()
{
	if(class_CVar)
		(*javaEnv)->UnregisterNatives(javaEnv, class_CVar);

	(*javaEnv)->DeleteLocalRef(javaEnv, class_CVar);
}

// ====================================================================================

/*
 * Class:     xreal_Engine
 * Method:    print
 * Signature: (Ljava/lang/String;)V
 */
void JNICALL Java_xreal_Engine_print(JNIEnv * env, jclass cls, jstring js)
{
	char            string[MAXPRINTMSG];

	if(js == NULL)
		return;

	ConvertJavaString(string, js, sizeof(string));

	Com_Printf("%s", string);
}

/*
 * Class:     xreal_Engine
 * Method:    error
 * Signature: (Ljava/lang/String;)V
 */
void JNICALL Java_xreal_Engine_error(JNIEnv *env, jclass cls, jstring js)
{
	char            string[MAXPRINTMSG];

	if(js == NULL)
		return;

	ConvertJavaString(string, js, sizeof(string));

	Com_Error(ERR_DROP, "%s", string);
}

/*
 * Class:     xreal_Engine
 * Method:    getTimeInMilliseconds
 * Signature: ()I
 */
jint JNICALL Java_xreal_Engine_getTimeInMilliseconds(JNIEnv *env, jclass cls)
{
	return Sys_Milliseconds();
}


/*
 * Class:     xreal_Engine
 * Method:    getConsoleArgc
 * Signature: ()I
 */
jint JNICALL Java_xreal_Engine_getConsoleArgc(JNIEnv *env, jclass cls)
{
	return Cmd_Argc();
}

/*
 * Class:     xreal_Engine
 * Method:    getConsoleArgv
 * Signature: (I)Ljava/lang/String;
 */
jstring JNICALL Java_xreal_Engine_getConsoleArgv(JNIEnv *env, jclass cls, jint arg)
{
	return (*env)->NewStringUTF(env, Cmd_Argv(arg));
}

/*
 * Class:     xreal_Engine
 * Method:    getConsoleArgs
 * Signature: ()Ljava/lang/String;
 */
jstring JNICALL Java_xreal_Engine_getConsoleArgs(JNIEnv *env, jclass cls)
{
	return (*env)->NewStringUTF(env, Cmd_Args());
}

/*
 * Class:     xreal_Engine
 * Method:    sendConsoleCommand
 * Signature: (ILjava/lang/String;)V
 */
void JNICALL Java_xreal_Engine_sendConsoleCommand(JNIEnv *env, jclass cls, jint jexec_when, jstring jtext)
{
	char           *text;

	text = (char *)((*env)->GetStringUTFChars(env, jtext, 0));

	Cbuf_ExecuteText(jexec_when, text);

	(*env)->ReleaseStringUTFChars(env, jtext, text);
}

// handle to Engine class
static jclass   class_Engine;
static JNINativeMethod Engine_methods[] = {
	{"print", "(Ljava/lang/String;)V", Java_xreal_Engine_print},
	{"error", "(Ljava/lang/String;)V", Java_xreal_Engine_error},
	{"getTimeInMilliseconds", "()I", Java_xreal_Engine_getTimeInMilliseconds},
	{"getConsoleArgc", "()I", Java_xreal_Engine_getConsoleArgc},
	{"getConsoleArgv", "(I)Ljava/lang/String;", Java_xreal_Engine_getConsoleArgv},
	{"getConsoleArgs", "()Ljava/lang/String;", Java_xreal_Engine_getConsoleArgs},
	{"sendConsoleCommand", "(ILjava/lang/String;)V", Java_xreal_Engine_sendConsoleCommand},
};

void Engine_javaRegister()
{
	class_Engine = (*javaEnv)->FindClass(javaEnv, "xreal/Engine");
	if(CheckException() || !class_Engine)
	{
		Com_Error(ERR_FATAL, "Couldn't find xreal.Engine");
	}

	(*javaEnv)->RegisterNatives(javaEnv, class_Engine, Engine_methods, sizeof(Engine_methods) / sizeof(Engine_methods[0]));
	if(CheckException())
	{
		Com_Error(ERR_FATAL, "Couldn't register native methods for xreal.Engine");
	}
}


void Engine_javaDetach()
{
	if(class_Engine)
		(*javaEnv)->UnregisterNatives(javaEnv, class_Engine);

	(*javaEnv)->DeleteLocalRef(javaEnv, class_Engine);
}

// ====================================================================================


/**
 * @brief Convert a Java string (which is Unicode) to reasonable 7-bit ASCII.
 *
 * @author Berry Pederson
 */
void ConvertJavaString(char *dest, jstring jstr, int destsize)
{
	if(!jstr)
	{
		Com_Error(ERR_FATAL, "ConvertJavaString: NULL src");
	}

	if(destsize < 1)
	{
		Com_Error(ERR_FATAL, "ConvertJavaString: destsize < 1");
	}

	// table for translating accented latin-1 characters to closest non-accented chars
	// Icelandic thorn and eth are difficult, so I just used an asterisk
	// German sz ligature � is also difficult, chose to just put in 's'
	// AE ligatures are just replaced with *
	// perhaps some sort of multi-character substitution scheme would be helpful
	//
	// this translation table starts at decimal 192 (capital A, grave accent: �)
	static char    *translateTable = "AAAAAA*CEEEEIIIIDNOOOOOxOUUUUY*saaaaaa*ceeeeiiii*nooooo/ouuuuy*y";

	jsize           jStrLen;
	const jchar    *unicodeChars;

//  char           *result;
	int             i;
	char           *p;

	jStrLen = Q_min((*javaEnv)->GetStringLength(javaEnv, jstr), destsize);

	p = dest;					// = q2java_gi.TagMalloc(jStrLen + 1, TAG_GAME);
	unicodeChars = (*javaEnv)->GetStringChars(javaEnv, jstr, NULL);

	for(i = 0; i < jStrLen; i++)
	{
		jchar           ch = unicodeChars[i];

		if(ch < 192)
			*p++ = (char)ch;
		else
		{
			if(ch < 256)
				*p++ = translateTable[ch - 192];
			else
				*p++ = '*';
		}

	}

	(*javaEnv)->ReleaseStringChars(javaEnv, jstr, unicodeChars);
	*p = 0;

//  return result;
}

/**
 * @author Berry Pederson
 */
qboolean CheckException_(char *filename, int linenum)
{
	jthrowable      ex;

	ex = (*javaEnv)->ExceptionOccurred(javaEnv);
	if(!ex)
		return qfalse;

	(*javaEnv)->ExceptionClear(javaEnv);

	Com_Printf("%s line: %d\n-----------------\n", filename, linenum);

	(*javaEnv)->CallVoidMethod(javaEnv, ex, method_Throwable_printStackTrace);

	return qtrue;
}



// ====================================================================================


void JVM_Shutdown(void)
{
	Com_Printf("------- JVM_Shutdown() -------\n");

	if(!javaEnv)
	{
		Com_Printf("Can't stop Java VM, javaEnv pointer was null\n");
		return;
	}

//  Java_G_ShutdownGame(qfalse);

	CVar_javaDetach();
	Engine_javaDetach();
	Misc_javaDetach();

	if(javaVMIsOurs)
	{
		JavaVM         *jvm;

		(*javaEnv)->GetJavaVM(javaEnv, &jvm);
		if((*jvm)->DestroyJavaVM(jvm))
		{
			Com_Printf("Error destroying Java VM\n");
		}
		else
		{
			Com_Printf("Java VM Destroyed\n");
		}

		JVM_JNI_Shutdown();
	}
}

void JVM_Init(void)
{
	JavaVM         *jvm;
	jsize           nVMs;		// number of VM's active
	jint            res;
	jclass          cls;
	jmethodID       mid;
	jstring         jstr;
	jclass          stringClass;
	jobjectArray    args;
	int             jdkVersion;

	JavaVMInitArgs  vm_args;
	JavaVMOption    options[1];

	char           *ospath;
	char            mainClassPath[MAX_QPATH];

	Com_Printf("------- JVM_Init() -------\n");

	jvm_javaLib = Cvar_Get("jvm_javaLib", DEFAULT_JAVA_LIB, CVAR_ARCHIVE);

	//options[0].optionString = "-Djava.class.path=" FS_G;// USER_CLASSPATH;
	Com_sprintf(mainClassPath, sizeof(mainClassPath), "-Djava.class.path=%s",
				FS_BuildOSPath(Cvar_VariableString("fs_basepath"), Cvar_VariableString("fs_game"), "classes"));
	options[0].optionString = mainClassPath;

	vm_args.version = JNI_VERSION_1_2;
	vm_args.options = options;
	vm_args.nOptions = 1;
	vm_args.ignoreUnrecognized = JNI_TRUE;

	if(!JVM_JNI_Init())
	{
		Com_Error(ERR_FATAL, "JNI initialization failed");
	}

	// look for an existing VM
	if(QJNI_GetCreatedJavaVMs(&jvm, 1, &nVMs))
	{
		Com_Error(ERR_FATAL, "Search for existing VM's failed");
	}

	if(nVMs)
	{
		if((*jvm)->AttachCurrentThread(jvm, (void **)&javaEnv, NULL))
		{
			Com_Error(ERR_FATAL, "Couldn't attach to existing VM");
		}

		javaVMIsOurs = qfalse;
		javaVM = jvm;

		Com_Printf("attached to existing Java VM\n");
	}
	else
	{
		// Create the Java VM
		res = QJNI_CreateJavaVM(&jvm, (void **)&javaEnv, &vm_args);
		if(res < 0)
		{
			Com_Error(ERR_FATAL, "Can't create Java VM");
		}

		javaVM = jvm;

		Com_Printf("created new Java VM\n");
	}

	if(!javaVM)
	{
		Com_Error(ERR_FATAL, "JVM_Init failed");
	}

	// finally register the needed core modules
	Misc_javaRegister();
	Engine_javaRegister();
	CVar_javaRegister();
}



#endif							//defined(USE_JAVA
