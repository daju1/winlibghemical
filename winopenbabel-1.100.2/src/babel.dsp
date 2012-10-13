# Microsoft Developer Studio Project File - Name="babel" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=babel - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "babel.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "babel.mak" CFG="babel - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "babel - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "babel - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "babel - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "babel___Win32_Release"
# PROP BASE Intermediate_Dir "babel___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "babel___Win32_Release"
# PROP Intermediate_Dir "babel___Win32_Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x419 /d "NDEBUG"
# ADD RSC /l 0x419 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"../../babel.exe"

!ELSEIF  "$(CFG)" == "babel - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "babel___Win32_Debug"
# PROP BASE Intermediate_Dir "babel___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "babel___Win32_Debug"
# PROP Intermediate_Dir "babel___Win32_Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /w /W0 /Gm /GX /ZI /Od /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D "_ASSERT" /D "_ASSERTE" /D "_RPTn" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x419 /d "_DEBUG"
# ADD RSC /l 0x419 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "babel - Win32 Release"
# Name "babel - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\alchemy.cpp
# End Source File
# Begin Source File

SOURCE=.\amber.cpp
# End Source File
# Begin Source File

SOURCE=.\atom.cpp
# End Source File
# Begin Source File

SOURCE=.\balst.cpp
# End Source File
# Begin Source File

SOURCE=.\base.cpp
# End Source File
# Begin Source File

SOURCE=.\bgf.cpp
# End Source File
# Begin Source File

SOURCE=.\binary.cpp
# End Source File
# Begin Source File

SOURCE=.\bitvec.cpp
# End Source File
# Begin Source File

SOURCE=.\bond.cpp
# End Source File
# Begin Source File

SOURCE=.\box.cpp
# End Source File
# Begin Source File

SOURCE=.\c3d.cpp
# End Source File
# Begin Source File

SOURCE=.\cacao.cpp
# End Source File
# Begin Source File

SOURCE=.\cache.cpp
# End Source File
# Begin Source File

SOURCE=.\car.cpp
# End Source File
# Begin Source File

SOURCE=.\ccc.cpp
# End Source File
# Begin Source File

SOURCE=.\chains.cpp
# End Source File
# Begin Source File

SOURCE=.\chdrw.cpp
# End Source File
# Begin Source File

SOURCE=.\chemtool.cpp
# End Source File
# Begin Source File

SOURCE=.\chiral.cpp
# End Source File
# Begin Source File

SOURCE=.\cml.cpp
# End Source File
# Begin Source File

SOURCE=.\crk.cpp
# End Source File
# Begin Source File

SOURCE=.\csr.cpp
# End Source File
# Begin Source File

SOURCE=.\cssr.cpp
# End Source File
# Begin Source File

SOURCE=.\data.cpp
# End Source File
# Begin Source File

SOURCE=.\dmol.cpp
# End Source File
# Begin Source File

SOURCE=.\feat.cpp
# End Source File
# Begin Source File

SOURCE=.\fh.cpp
# End Source File
# Begin Source File

SOURCE=.\fileformat.cpp
# End Source File
# Begin Source File

SOURCE=.\gamess.cpp
# End Source File
# Begin Source File

SOURCE=.\gaussian.cpp
# End Source File
# Begin Source File

SOURCE=.\generic.cpp
# End Source File
# Begin Source File

SOURCE=.\ghemical.cpp
# End Source File
# Begin Source File

SOURCE=.\grid.cpp
# End Source File
# Begin Source File

SOURCE=.\gromos96.cpp
# End Source File
# Begin Source File

SOURCE=.\hin.cpp
# End Source File
# Begin Source File

SOURCE=.\jaguar.cpp
# End Source File
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# Begin Source File

SOURCE=.\matrix.cpp
# End Source File
# Begin Source File

SOURCE=.\math\matrix3x3.cpp
# End Source File
# Begin Source File

SOURCE=.\mdl.cpp
# End Source File
# Begin Source File

SOURCE=.\mm3.cpp
# End Source File
# Begin Source File

SOURCE=.\mmod.cpp
# End Source File
# Begin Source File

SOURCE=.\mol.cpp
# End Source File
# Begin Source File

SOURCE=.\mol2.cpp
# End Source File
# Begin Source File

SOURCE=.\molchrg.cpp
# End Source File
# Begin Source File

SOURCE=.\molvector.cpp
# End Source File
# Begin Source File

SOURCE=.\mopac.cpp
# End Source File
# Begin Source File

SOURCE=.\mpqc.cpp
# End Source File
# Begin Source File

SOURCE=.\nwchem.cpp
# End Source File
# Begin Source File

SOURCE=.\oberror.cpp
# End Source File
# Begin Source File

SOURCE=.\obutil.cpp
# End Source File
# Begin Source File

SOURCE=.\parsmart.cpp
# End Source File
# Begin Source File

SOURCE=.\parsmi.cpp
# End Source File
# Begin Source File

SOURCE=.\patty.cpp
# End Source File
# Begin Source File

SOURCE=.\pdb.cpp
# End Source File
# Begin Source File

SOURCE=.\phmodel.cpp
# End Source File
# Begin Source File

SOURCE=.\povray.cpp
# End Source File
# Begin Source File

SOURCE=.\pqs.cpp
# End Source File
# Begin Source File

SOURCE=.\qchem.cpp
# End Source File
# Begin Source File

SOURCE=.\rand.cpp
# End Source File
# Begin Source File

SOURCE=.\report.cpp
# End Source File
# Begin Source File

SOURCE=.\residue.cpp
# End Source File
# Begin Source File

SOURCE=.\ring.cpp
# End Source File
# Begin Source File

SOURCE=.\rotor.cpp
# End Source File
# Begin Source File

SOURCE=.\shelx.cpp
# End Source File
# Begin Source File

SOURCE=.\smi.cpp
# End Source File
# Begin Source File

SOURCE=.\snprintf.c
# End Source File
# Begin Source File

SOURCE=.\strncasecmp.c
# End Source File
# Begin Source File

SOURCE=.\tinker.cpp
# End Source File
# Begin Source File

SOURCE=.\tokenst.cpp
# End Source File
# Begin Source File

SOURCE=.\typer.cpp
# End Source File
# Begin Source File

SOURCE=.\unichem.cpp
# End Source File
# Begin Source File

SOURCE=.\math\vector3.cpp
# End Source File
# Begin Source File

SOURCE=.\viewmol.cpp
# End Source File
# Begin Source File

SOURCE=.\xed.cpp
# End Source File
# Begin Source File

SOURCE=.\xyz.cpp
# End Source File
# Begin Source File

SOURCE=.\zindo.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\aromatic.h
# End Source File
# Begin Source File

SOURCE=.\atomtyp.h
# End Source File
# Begin Source File

SOURCE=.\babelconfig.h
# End Source File
# Begin Source File

SOURCE=.\base.h
# End Source File
# Begin Source File

SOURCE=.\binary.h
# End Source File
# Begin Source File

SOURCE=.\bitvec.h
# End Source File
# Begin Source File

SOURCE=.\bondtyper.h
# End Source File
# Begin Source File

SOURCE=.\chains.h
# End Source File
# Begin Source File

SOURCE=.\chiral.h
# End Source File
# Begin Source File

SOURCE=.\crk.h
# End Source File
# Begin Source File

SOURCE=.\data.h
# End Source File
# Begin Source File

SOURCE=.\element.h
# End Source File
# Begin Source File

SOURCE=.\extable.h
# End Source File
# Begin Source File

SOURCE=.\fileformat.h
# End Source File
# Begin Source File

SOURCE=.\generic.h
# End Source File
# Begin Source File

SOURCE=.\grid.h
# End Source File
# Begin Source File

SOURCE=.\isotope.h
# End Source File
# Begin Source File

SOURCE=.\matrix.h
# End Source File
# Begin Source File

SOURCE=.\math\matrix3x3.h
# End Source File
# Begin Source File

SOURCE=.\mol.h
# End Source File
# Begin Source File

SOURCE=.\molchrg.h
# End Source File
# Begin Source File

SOURCE=.\molvector.h
# End Source File
# Begin Source File

SOURCE=.\oberror.h
# End Source File
# Begin Source File

SOURCE=.\obifstream.h
# End Source File
# Begin Source File

SOURCE=.\obutil.h
# End Source File
# Begin Source File

SOURCE=.\parsmart.h
# End Source File
# Begin Source File

SOURCE=.\patty.h
# End Source File
# Begin Source File

SOURCE=.\phmodel.h
# End Source File
# Begin Source File

SOURCE=.\phmodeldata.h
# End Source File
# Begin Source File

SOURCE=.\resdata.h
# End Source File
# Begin Source File

SOURCE=.\ring.h
# End Source File
# Begin Source File

SOURCE=.\rotor.h
# End Source File
# Begin Source File

SOURCE=.\smi.h
# End Source File
# Begin Source File

SOURCE=.\snprintf.h
# End Source File
# Begin Source File

SOURCE=.\typer.h
# End Source File
# Begin Source File

SOURCE=.\types.h
# End Source File
# Begin Source File

SOURCE=.\math\vector3.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
