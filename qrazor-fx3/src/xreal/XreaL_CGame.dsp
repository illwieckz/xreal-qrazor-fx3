# Microsoft Developer Studio Project File - Name="XreaL_CGame" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=XreaL_CGame - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "XreaL_CGame.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "XreaL_CGame.mak" CFG="XreaL_CGame - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "XreaL_CGame - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "XreaL_CGame - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "XreaL_CGame - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release\cgame"
# PROP Intermediate_Dir "Release\cgame"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I ".." /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../bins/Release/xreal/CGame.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "XreaL_CGame - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug\cgame"
# PROP Intermediate_Dir "Debug\cgame"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I ".." /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR"Debug/cgame/browse/" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo /o"Debug\cgame\browse\XreaL_CGame.bsc"
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib winmm.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../bins/xreal/CGame.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "XreaL_CGame - Win32 Release"
# Name "XreaL_CGame - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\cg_decal.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_entity.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_event.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_explosion.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_hud.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_inv.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_light.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_main.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_muzzleflash.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_particle.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_player.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_pred.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_screen.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_tent.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_view.cxx
# End Source File
# Begin Source File

SOURCE=.\cg_weapon.cxx
# End Source File
# Begin Source File

SOURCE=.\cgame.def
# End Source File
# Begin Source File

SOURCE=.\x_shared.cxx
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\cg_local.h
# End Source File
# Begin Source File

SOURCE=.\cg_public.h
# End Source File
# Begin Source File

SOURCE=.\x_shared.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
