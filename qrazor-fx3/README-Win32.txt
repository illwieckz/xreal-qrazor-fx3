_.,:oO-| XreaL for Win32 installation (v0.1) |-Oo:,._

First of all:
The XreaL installation is not hard, but also not easy. You might encounter
problems on your system while others can get it running flawlessly. This
is because the Win32 port was a quick one and all in all the XreaL engine
is still in pre-alpha state. Read 'pre-alpha' as 'fragile'.
The people on the irc channel #xreal on freenode (irc.freenode.net:6667)
are always willing to help, and in one step of the installation (the last
one) you have to visit the XreaL IRC channel anyways. But try to follow the
steps in this readme first before contacting the XreaL team. The purpose
of this file is to give you all the information you need, because we noticed
that there is a lot of interest in the XreaL engine from Win32 users.

! Please note that all single quotation marks are just there for better       !
! readability and should not be typed when you are required to type something !
! somewhere.                                                                  !

--

What you need:
- Bloodshed Dev-C++ (http://www.bloodshed.net/devcpp.html)
- Python (http://www.python.org)
- Scons (http://www.scons.org)
- Doom 3 (your local game shop) (only for techdemo testing, 
will be replaced by a stand-a-lone game)

--

Setting it up:
^^^^^^^^^^^^^^
Install Dev-C++ and start it. Select Tools --> Check for Updates/Packages.
In the update dialogue search on the 2 update servers for boost, libpng, 
libjpeg, libz and OpenAL. Notice that libz has to be installed before 
installing libjpeg or libpng. The image libraries rely on libz and the 
installer of those libraries will cancel if libz is not found. So install
libz first, and after that search for the other packages.

Now install python, and after that install scons.
Additionally you have to check the systems PATH environment variable. Right
click on 'My Computer' on your desktop or choose 'System' in the Windows 
control panel. There you choose the 'Advanced' tab and there 
'environment variables' (See * at the end of the document). What we are 
looking for is 'Systemvariables' and there the PATH entry. So choose it and
'Edit' it. The string should have 'c:\python24;c:\dev-cpp\bin' in it. If
something is missing then add it. Python is needed for scons and the scons.bat
is in that python directory (btw. if you have Python 2.3 installed it should be
python23 instead of python24). The dev-cpp dir is the one with the compiler and
it will be able to find its lib and include dirs automatically. This, paired with
the package update system of dev-cpp, makes it pretty easy to install and setup.

Now open up a commandline (Start->execute(?), type 'cmd' in the dialogue and 
hit enter). CD to your qrazor source dir and execute scons.

--

A scons primer:
^^^^^^^^^^^^^^^
Scons is a make like system. This means that it can read scripts with 'rules'
and then execute these rules according to commandline parameters supplied and
the state of files that are used. The difference is that scons is using python
as a scripting language instead of shell scripts. This makes it a little more 
flexible, expandable and all in all its easier to use python than shell scripts
in my humble opinion :). So, let's look at some commands that can be used with
our little project:
- 'scons -c'                = Pretty important. You can delete all the compiled 
                              objects and temporary files, thus 'c'learing the 
                              build. Just in case you want te recompile 
                              everything.
- 'scons simd=<option>'     = Where <option> is either 3dnow, sse or none. Leave 
                              it to the default ('none') on your first compile.
- 'scons PKGDATADIR=<path>' = This is where the engine looks for data files.
                              By default this is '.' or the current dir. And the whole
                              stuff needs to be copied there after a compile to 
                              run. So if you rather want your XreaL to reside
                              elsewhere on your HDD you better suppy a path here.
                              
Please notice that SCons is able to remember its settings. So you don't have to supply
the same commandline everytime you compile. This also means that you have to turn off 
settings explicitly. So if you supplied simd=sse some time earlier, and now you want 
to compile without, you have to supply simd=none as a commandline parameter to scons.
Also you can supply more than one commandline parameter at once 
(e.g. like 'scons simd=3dnow PKGDATADIR=C:\Path\To\QRazor-FX').

--

It compiled? Woohoo!
^^^^^^^^^^^^^^^^^^^^
All the XreaL stuff is now in 'qrazor-fx3\bin'. You also have to copy the pk4 files from 
your 'Doom3\base' dir over to 'qrazor-fx3\bin\xreal'. Now leech a new pak file from our
sf.net files directory. http://xreal.sourceforge.net/files/xpak000.pk4
It contains a font, some maps and a config file. As i tried this Windows was so nice to
rename the file from xpak000.pk4 to xpak000.zip. Thats not intended so check if the name
is correct and rename the file if neccessary. It also goes to the directory with the
Doom3 pak files (guess you knew that ;)).
So, what I do now is I open 2 more commandlines. I CD to the 'qrazor-fx3\bin' dir in both.
Now enter 'QRazor-FX-Server +map game/mp/d3dm1' in one of the commandlines. The server 
should start up loading the supplied map. (theres also d3dm2 and d3dm3 in that pak file)
In the other commandline execute 'QRazor-FX-Client +set vid_mode 3' (3 is 640x480).
After some printouts the client window should pop up and it either shows the console
or you can pull it down with the console key (~ or ^). Type 'connect localhost' in the
console and qrazor should connect to the server. To move around use the Arrow keys and
the mouse. Enter 'noclip' into the console first or you cannot move.
That should be all for now. QRazor-FX is set up and running. Look into the xpak000.pk4
for the default.cfg and the keys that are bound there.

Cheers...
bj0ern (bj0ern@users.sourceforge.net)


*) This might be named different in the english Windows XP version. 
   I just have the german one so I have to guess what it is named in
   the english one. If you have the english version you can send me the 
   right names to the e-mail address supplied at the end of this readme.
