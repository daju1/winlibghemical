lib_LTLIBRARIES = libghemical.la

libghemical_la_SOURCES = \
		atom.cpp bond.cpp \
		model.cpp engine.cpp \
		typerule.cpp utility.cpp \
		seqbuild.cpp \
		eng1_qm.cpp \
		eng1_qm_mpqc.cpp eng1_qm_mopac.cpp \
		eng1_mm.cpp \
		eng1_mm_tripos52.cpp tab_mm_tripos52.cpp \
		eng1_mm_default.cpp tab_mm_default.cpp \
		eng1_mm_prmfit.cpp tab_mm_prmfit.cpp \
		eng2_qm_mm.cpp \
		eng1_sf.cpp \
		eng2_mm_sf.cpp \
		notice.cpp \
		conjgrad.cpp \
		intcrd.cpp search.cpp \
		geomopt.cpp moldyn.cpp \
		pop_ana.cpp resonance.cpp

libghemicalincludedir = $(includedir)/ghemical

##libghemical_HEADERS = \	;not working, says invalid variable...
##libghemical_la_HEADERS = \	;not working, says invalid variable...
libghemicalinclude_HEADERS = \
		libconfig.h libdefine.h typedef.h v3d.h \
		atom.h bond.h \
		model.h engine.h \
		typerule.h utility.h \
		seqbuild.h \
		eng1_qm.h \
		eng1_qm_mpqc.h eng1_qm_mopac.h \
		eng1_mm.h \
		eng1_mm_tripos52.h tab_mm_tripos52.h \
		eng1_mm_default.h tab_mm_default.h \
		eng1_mm_prmfit.h tab_mm_prmfit.h \
		eng2_qm_mm.h \
		eng1_sf.h \
		eng2_mm_sf.h \
		notice.h \
		conjgrad.h \
		intcrd.h search.h \
		geomopt.h moldyn.h \
		pop_ana.h resonance.h

##libghemical_CPPFLAGS += -I$(top_builddir)/src @SC_CPPFLAGS@		;this is not working...
##libghemical_la_CPPFLAGS += -I$(top_builddir)/src @SC_CPPFLAGS@	;this is not working...
##INCLUDES += -I$(top_builddir)/src @SC_CPPFLAGS@			;this is working but is obsolete...
AM_CPPFLAGS += -I$(top_builddir)/src @SC_CPPFLAGS@

## the -I$(top_builddir)/src above just yields -I../src ; how to get an absolute path?

EXTRA_DIST = Doxyfile
