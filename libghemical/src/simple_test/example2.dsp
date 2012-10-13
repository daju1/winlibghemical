# Microsoft Developer Studio Project File - Name="example2" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=example2 - Win32 Release
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "example2.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "example2.mak" CFG="example2 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "example2 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "example2 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "example2 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "example2 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"../../../../example2.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "example2 - Win32 Release"
# Name "example2 - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\atom.cpp
# End Source File
# Begin Source File

SOURCE=..\bond.cpp
# End Source File
# Begin Source File

SOURCE=..\conjgrad.cpp
# End Source File
# Begin Source File

SOURCE=..\eng1_mm.cpp
# End Source File
# Begin Source File

SOURCE=..\eng1_mm_default.cpp
# End Source File
# Begin Source File

SOURCE=..\eng1_mm_prmfit.cpp
# End Source File
# Begin Source File

SOURCE=..\eng1_mm_tripos52.cpp
# End Source File
# Begin Source File

SOURCE=..\engine.cpp
# End Source File
# Begin Source File

SOURCE=.\example2.cpp
# End Source File
# Begin Source File

SOURCE=..\fileio.cpp
# End Source File
# Begin Source File

SOURCE=..\geomopt.cpp
# End Source File
# Begin Source File

SOURCE=..\intcrd.cpp
# End Source File
# Begin Source File

SOURCE=..\model.cpp
# End Source File
# Begin Source File

SOURCE=..\moldyn.cpp
# End Source File
# Begin Source File

SOURCE=..\notice.cpp
# End Source File
# Begin Source File

SOURCE=..\resonance.cpp
# End Source File
# Begin Source File

SOURCE=..\search.cpp
# End Source File
# Begin Source File

SOURCE=..\seqbuild.cpp
# End Source File
# Begin Source File

SOURCE=..\tab_mm_default.cpp
# End Source File
# Begin Source File

SOURCE=..\tab_mm_prmfit.cpp
# End Source File
# Begin Source File

SOURCE=..\tab_mm_tripos52.cpp
# End Source File
# Begin Source File

SOURCE=..\typerule.cpp
# End Source File
# Begin Source File

SOURCE=..\utility.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\atom.h
# End Source File
# Begin Source File

SOURCE=..\bond.h
# End Source File
# Begin Source File

SOURCE=..\conjgrad.h
# End Source File
# Begin Source File

SOURCE=..\eng1_mm.h
# End Source File
# Begin Source File

SOURCE=..\eng1_mm_default.h
# End Source File
# Begin Source File

SOURCE=..\eng1_mm_prmfit.h
# End Source File
# Begin Source File

SOURCE=..\eng1_mm_tripos52.h
# End Source File
# Begin Source File

SOURCE=..\engine.h
# End Source File
# Begin Source File

SOURCE=.\example2.h
# End Source File
# Begin Source File

SOURCE=..\geomopt.h
# End Source File
# Begin Source File

SOURCE=..\intcrd.h
# End Source File
# Begin Source File

SOURCE=..\libconfig.h
# End Source File
# Begin Source File

SOURCE=..\model.h
# End Source File
# Begin Source File

SOURCE=..\moldyn.h
# End Source File
# Begin Source File

SOURCE=..\notice.h
# End Source File
# Begin Source File

SOURCE=..\resonance.h
# End Source File
# Begin Source File

SOURCE=..\search.h
# End Source File
# Begin Source File

SOURCE=..\seqbuild.h
# End Source File
# Begin Source File

SOURCE=..\tab_mm_default.h
# End Source File
# Begin Source File

SOURCE=..\tab_mm_prmfit.h
# End Source File
# Begin Source File

SOURCE=..\tab_mm_tripos52.h
# End Source File
# Begin Source File

SOURCE=..\typerule.h
# End Source File
# Begin Source File

SOURCE=..\utility.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
