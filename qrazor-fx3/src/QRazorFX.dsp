# Microsoft Developer Studio Project File - Name="QRazorFX" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=QRazorFX - Win32 Dedicated Client Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "QRazorFX.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "QRazorFX.mak" CFG="QRazorFX - Win32 Dedicated Client Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "QRazorFX - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "QRazorFX - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "QRazorFX - Win32 Dedicated Server Debug" (based on "Win32 (x86) Application")
!MESSAGE "QRazorFX - Win32 Dedicated Server Release" (based on "Win32 (x86) Application")
!MESSAGE "QRazorFX - Win32 Dedicated Client Debug" (based on "Win32 (x86) Application")
!MESSAGE "QRazorFX - Win32 Dedicated Client Release" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\combined"
# PROP Intermediate_Dir "Release\combined"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib zlibstat.lib openal32.lib alut.lib alu.lib /nologo /subsystem:windows /machine:I386 /out:"../bins/Release/QRazorFX.exe"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug\combined"
# PROP Intermediate_Dir "Debug\combined"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug/browse/QRazorFX.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib zlibstat.lib openal32.lib alut.lib alu.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../bins/QRazorFX.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug\server"
# PROP Intermediate_Dir "Debug\server"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEDICATED_ONLY" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug\server\browse\QRazorFX.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib zlibstat.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../bins/Qrazor-FX-server.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\server"
# PROP Intermediate_Dir "Release\server"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "DEDICATED_ONLY" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib zlibstat.lib /nologo /subsystem:windows /machine:I386 /out:"../bins/Release/Qrazor-FX-server.exe"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug\client"
# PROP Intermediate_Dir "Debug\client"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug\client\browse\QRazorFX.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib zlibstat.lib openal32.lib alut.lib alu.lib /nologo /subsystem:windows /debug /machine:I386 /out:"../bins/Qrazor-FX-client.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\client"
# PROP Intermediate_Dir "Release\client"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 winmm.lib wsock32.lib kernel32.lib user32.lib gdi32.lib zlibstat.lib openal32.lib alut.lib alu.lib /nologo /subsystem:windows /machine:I386 /out:"../bins/Qrazor-FX-client.exe"

!ENDIF 

# Begin Target

# Name "QRazorFX - Win32 Release"
# Name "QRazorFX - Win32 Debug"
# Name "QRazorFX - Win32 Dedicated Server Debug"
# Name "QRazorFX - Win32 Dedicated Server Release"
# Name "QRazorFX - Win32 Dedicated Client Debug"
# Name "QRazorFX - Win32 Dedicated Client Release"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "Client"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cl_cgame.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_console.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_input.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_keys.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_main.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_null.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_parse.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_sound.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\cl_ui.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "Misc"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\cmap.cxx
# End Source File
# Begin Source File

SOURCE=.\cmd.cxx
# End Source File
# Begin Source File

SOURCE=.\common.cxx
# End Source File
# Begin Source File

SOURCE=.\crc.cxx
# End Source File
# Begin Source File

SOURCE=.\cvar.cxx
# End Source File
# Begin Source File

SOURCE=.\huff.cxx
# End Source File
# Begin Source File

SOURCE=.\md4.cxx
# End Source File
# Begin Source File

SOURCE=.\mem.cxx
# End Source File
# Begin Source File

SOURCE=.\pmove.cxx
# End Source File
# Begin Source File

SOURCE=.\sha1.cxx
# End Source File
# Begin Source File

SOURCE=.\unzip.cxx
# End Source File
# Begin Source File

SOURCE=.\vfs.cxx
# End Source File
# Begin Source File

SOURCE=.\x_shared.cxx
# End Source File
# End Group
# Begin Group "Net"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net_chan.cxx
# End Source File
# Begin Source File

SOURCE=.\net_msg.cxx
# End Source File
# End Group
# Begin Group "Server"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sv_ccmds.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_ents.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_game.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_init.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_main.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_map.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_null.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_send.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_user.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sv_world.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Win32"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\in_win.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sys_win.cxx
# End Source File
# Begin Source File

SOURCE=.\vid_dll.cxx

!IF  "$(CFG)" == "QRazorFX - Win32 Release"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Server Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Debug"

!ELSEIF  "$(CFG)" == "QRazorFX - Win32 Dedicated Client Release"

!ENDIF 

# End Source File
# End Group
# Begin Group "Map"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\map_main.cxx
# PROP Exclude_From_Build 1
# End Source File
# Begin Source File

SOURCE=.\map_null.cxx
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\anorms.h
# End Source File
# Begin Source File

SOURCE=.\cbsp.h
# End Source File
# Begin Source File

SOURCE=.\XReal\cg_public.h
# End Source File
# Begin Source File

SOURCE=.\cl_console.h
# End Source File
# Begin Source File

SOURCE=.\cl_keys.h
# End Source File
# Begin Source File

SOURCE=.\cl_local.h
# End Source File
# Begin Source File

SOURCE=.\client.h
# End Source File
# Begin Source File

SOURCE=.\cmap.h
# End Source File
# Begin Source File

SOURCE=.\cmd.h
# End Source File
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\crc.h
# End Source File
# Begin Source File

SOURCE=.\cvar.h
# End Source File
# Begin Source File

SOURCE=.\files.h
# End Source File
# Begin Source File

SOURCE=.\XReal\g_public.h
# End Source File
# Begin Source File

SOURCE=.\glimp_wgl.h
# End Source File
# Begin Source File

SOURCE=.\glob.h
# End Source File
# Begin Source File

SOURCE=.\img_jpg.h
# End Source File
# Begin Source File

SOURCE=.\img_png.h
# End Source File
# Begin Source File

SOURCE=.\img_tga.h
# End Source File
# Begin Source File

SOURCE=.\input.h
# End Source File
# Begin Source File

SOURCE=.\net_chan.h
# End Source File
# Begin Source File

SOURCE=.\net_msg.h
# End Source File
# Begin Source File

SOURCE=.\net_protocol.h
# End Source File
# Begin Source File

SOURCE=.\r_backend.h
# End Source File
# Begin Source File

SOURCE=.\r_local.h
# End Source File
# Begin Source File

SOURCE=.\r_public.h
# End Source File
# Begin Source File

SOURCE=.\ref.h
# End Source File
# Begin Source File

SOURCE=.\resource.h
# End Source File
# Begin Source File

SOURCE=.\rw_linux.h
# End Source File
# Begin Source File

SOURCE=.\server.h
# End Source File
# Begin Source File

SOURCE=.\sha1.h
# End Source File
# Begin Source File

SOURCE=.\sv_local.h
# End Source File
# Begin Source File

SOURCE=.\sys.h
# End Source File
# Begin Source File

SOURCE=.\XReal\ui_keycodes.h
# End Source File
# Begin Source File

SOURCE=.\XReal\ui_public.h
# End Source File
# Begin Source File

SOURCE=.\unzip.h
# End Source File
# Begin Source File

SOURCE=.\vfs.h
# End Source File
# Begin Source File

SOURCE=.\vid.h
# End Source File
# Begin Source File

SOURCE=.\winquake.h
# End Source File
# Begin Source File

SOURCE=.\XReal\x_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
