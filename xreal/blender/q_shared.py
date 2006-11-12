import sys, os, os.path, struct, string, math

import Blender, Blender.Scene
from Blender import Registry
from Blender.Window import DrawProgressBar
from Blender import Draw, BGL

import types

import textwrap

import logging 
reload(logging)

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


# our own logger class. it works just the same as a normal logger except
# all info messages get show. 
class Logger(logging.Logger):
	def __init__(self, name,level = logging.NOTSET):
		logging.Logger.__init__(self, name, level)
		
		self.has_warnings = False
		self.has_errors = False
		self.has_critical = False
	
	def info(self, msg, *args, **kwargs):
		apply(self._log,(logging.INFO, msg, args), kwargs)
		
	def warning(self, msg, *args, **kwargs):
		logging.Logger.warning(self, msg, *args, **kwargs)
		self.has_warnings = True
	
	def error(self, msg, *args, **kwargs):
		logging.Logger.error(self, msg, *args, **kwargs)
		self.has_errors = True
		
	def critical(self, msg, *args, **kwargs):
		logging.Logger.critical(self, msg, *args, **kwargs)
		self.has_errors = True
		
# should be able to make this print to stdout in realtime and save MESSAGES
# as well. perhaps also have a log to file option
class LogHandler(logging.StreamHandler):
	def __init__(self):
		logging.StreamHandler.__init__(self, sys.stdout)
		
		if "XreaL_IO-Suite_log" not in Blender.Text.Get():
			self.outtext = Blender.Text.New("XreaL_IO-Suite_log")
		else:
			self.outtext = Blender.Text.Get('XreaL_IO-Suite_log')
			self.outtext.clear()
			
		self.lastmsg = ''
		
	def emit(self, record):
		# print to stdout and  to a new blender text object
		msg = self.format(record)
		
		if msg == self.lastmsg:
			return
		
		self.lastmsg = msg
		self.outtext.write("%s\n" %msg)
		
		logging.StreamHandler.emit(self, record)

logging.setLoggerClass(Logger)
log = logging.getLogger('XreaL_IO-Suite')

handler = LogHandler()
formatter = logging.Formatter('%(levelname)s %(message)s')
handler.setFormatter(formatter)

log.addHandler(handler)
# set this to minimum output level. eg. logging.DEBUG, logging.WARNING, logging.ERROR
# logging.CRITICAL. logging.INFO will make little difference as these always get 
# output'd
log.setLevel(logging.WARNING)


class BlenderGui:
	def __init__(self):
		text = """A log has been written to a blender text window. Change this window type to 
a text window and you will be able to select the file XreaL_IO-Suite_log."""

		text = textwrap.wrap(text,40)
		text += ['']
		
		if log.has_critical:
			text += ['There were critical errors!!!!']
			
		elif log.has_errors:
			text += ['There were errors!']
			
		elif log.has_warnings:
			text += ['There were warnings']
			
		# add any more text before here
		text.reverse()
		
		self.msg = text
		
		Blender.Draw.Register(self.gui, self.event, self.button_event)
		
	def gui(self,):
		quitbutton = Blender.Draw.Button("Exit", 1, 0, 0, 100, 20, "Close Window")
		
		y = 35
		
		for line in self.msg:
			BGL.glRasterPos2i(10,y)
			Blender.Draw.Text(line)
			y+=15
			
	def event(self,evt, val):
		if evt == Blender.Draw.ESCKEY:
			Blender.Draw.Exit()
			return
	
	def button_event(self,evt):
		if evt == 1:
			Blender.Draw.Exit()
			return



