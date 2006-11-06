import sys, struct, string, math
from types import *

import os
from os import path

GAMEDIR = '/opt/XreaL/base/'
MAX_QPATH = 64

def asciiz(s):
	n = 0
	while(ord(s[n]) != 0):
		n = n + 1
	return s[0:n]

# strips the slashes from the back of a string
def StripPath(path):
	for c in range(len(path), 0, -1):
		if path[c-1] == "/" or path[c-1] == "\\":
			path = path[c:]
			break
	return path
	
# strips the model from path
def StripModel(path):
	for c in range(len(path), 0, -1):
		if path[c-1] == "/" or path[c-1] == "\\":
			path = path[:c]
			break
	return path

# strips file type extension
def StripExtension(name):
	if name.find('.') != -1:
		name = name[:name.find('.')]
	return name
	
# strips gamedir
def StripGamePath(name):
	if name[0:len(GAMEDIR)] == GAMEDIR:
		name = name[len(GAMEDIR):len(name)]
	return name

