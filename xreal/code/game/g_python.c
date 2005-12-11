/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
//
// g_python.c

#include <Python.h>
#include "g_local.h"


#define MAX_PYTHONFILE 32768

static PyObject *qagame_gprint(PyObject * self, PyObject * args)
{
	char           *s;

	if(!PyArg_ParseTuple(args, "s", &s))
		return NULL;

	G_Printf("%s", s);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *qagame_gprintln(PyObject * self, PyObject * args)
{
	char           *s;

	if(!PyArg_ParseTuple(args, "s", &s))
		return NULL;

	G_Printf("%s\n", s);

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *qagame_version(PyObject * self, PyObject * args)
{
	if(!PyArg_ParseTuple(args, ":version"))
		return NULL;

	return Py_BuildValue("s", GAMEVERSION);
}

static PyMethodDef qagameMethods[] = {
	{"gprint", qagame_gprint, METH_VARARGS, "Print to console."},
	{"gprintln", qagame_gprintln, METH_VARARGS, "Print to console and add EOL."},
	{"version", qagame_version, METH_VARARGS, "Return the game version."},
	{NULL, NULL, 0, NULL}
};

/*
============
G_InitPython
============
*/
void G_InitPython()
{
	G_Printf("------- Python Initialization -------\n");

	Py_Initialize();
	Py_InitModule("qagame", qagameMethods);

	//PyRun_SimpleString("print 'Hello Python World!'\n");
	
	//PyRun_SimpleString(	"import qagame\n"
	//					"qagame.gprint \"gamename: \", qagame.version()\n");
	
	//PyRun_SimpleString(	"import qagame\n"
	//				"qagame.gprint(qagame.version())\n");

	G_Printf("-----------------------------------\n");
}



/*
=================
G_ShutdownPython
=================
*/
void G_ShutdownPython()
{
	G_Printf("------- Python Finalization -------\n");

	Py_Finalize();

	G_Printf("-----------------------------------\n");
}


/*
=================
G_RunPythonScript
=================
*/
void G_RunPythonScript(gentity_t * ent, const char *filename)
{
	int             len;
	fileHandle_t    f;
	char            buf[MAX_PYTHONFILE];

	len = trap_FS_FOpenFile(filename, &f, FS_READ);
	if(!f)
	{
		trap_Printf(va(S_COLOR_RED "file not found: %s\n", filename));
		return;
	}
	
	if(len >= MAX_PYTHONFILE)
	{
		trap_Printf(va(S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_PYTHONFILE));
		trap_FS_FCloseFile(f);
		return;
	}

	trap_FS_Read(buf, len, f);
	buf[len] = 0;
	trap_FS_FCloseFile(f);
	
	PyRun_SimpleString(buf);
}

/*
=================
G_RunPythonFunction
=================
*/
void G_RunPythonFunction(const char *string)
{
	// TODO
}
