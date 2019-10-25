#-------------------------------------------------------------------------------
# Pd Spectral Toolkit Makefile
#
# Builds the Pd Spectral Toolkit on MacOS, Linux, or Windows
#
# Created By Tom Erbe on 06/25/19
# Copyright (c) 2019 Tom Erbe. All rights reserved.
#-------------------------------------------------------------------------------


#-------------------------------------------------------------------------------
# Build Configuration
#-------------------------------------------------------------------------------

# Pure Data Source Location
#-------------------------------------------------------------------------------
# use git to download a fresh copy of pd in the root level of this project:
# $ git clone https://github.com/pure-data/pure-data.git
PDSRC = ./pure-data/src

# MacOS Flags
#-------------------------------------------------------------------------------
# compiler ( cc, gcc or /usr/local/bin/gcc-9 )
MACOSCC = /usr/local/bin/gcc-9

# architecture ( i386 or x86_64 )
MACOSARCH = x86_64

# Windows Flags
#-------------------------------------------------------------------------------
# compiler ( note: you must edit the definition of VC to match your system )
WINDOWSVC = "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.15.26726"
# architecture ( change these for 64 or 32 )
WINDOWSLDIR = $(WINDOWSVC)\lib\x64


#-------------------------------------------------------------------------------
# - - - - - - - - - - - - DO NOT EDIT BELOW THIS LINE - - - - - - - - - - - - -
#-------------------------------------------------------------------------------


#-------------------------------------------------------------------------------
# Object File Lists
#-------------------------------------------------------------------------------

# macos / linux
#-------------------------------------------------------------------------------
OBJECTS = amptodb~.o amptomag~.o binindex~.o binmax~.o binmin~.o binmix~.o	\
	binmonitor~.o binsort~.o bintrim~.o bitsafe~.o blocksmooth~.o cartoamp~.o \
	cartodb~.o cartofreq~.o cartomag~.o cartophase~.o cartopolar~.o cmplxabs~.o \
	cmplxadd~.o cmplxdiv~.o cmplxmult~.o cmplxsqrt~.o cmplxsub~.o countwrap.o \
	ctltosig~.o dbtoamp~.o dbtomag~.o degtorad~.o degtoturn~.o dspbang~.o \
	freqsieve~.o freqtocar~.o freqtophase~.o freqtopolar~.o fundfreq~.o \
	harmprod~.o magtoamp~.o magtodb~.o magtrim~.o monitor~.o oscbank~.o \
	pafft~.o paifft~.o partconv~.o peaks~.o phaseaccum~.o phasedelta~.o \
	phasetofreq~.o piwrap~.o polartocar~.o polartofreq~.o radtodeg~.o \
	radtoturn~.o recip~.o rgbtable.o rotate~.o rounder~.o scale~.o sigtoctl~.o \
	softclip~.o pd_spectral_toolkit.o tabindex~.o terminal.o trunc~.o \
	turntodeg~.o turntorad~.o utility.o valleys~.o windower.o winfft~.o winifft~.o \
	nand~.o neq~.o nor~.o not~.o mod~.o and~.o leq~.o lt~.o eq~.o geq~.o gt~.o or~.o

# windows
#-------------------------------------------------------------------------------
WINOBJECTS = amptodb~.obj amptomag~.obj binindex~.obj binmax~.obj binmin~.obj binmix~.obj	\
	binmonitor~.obj binsort~.obj bintrim~.obj bitsafe~.obj blocksmooth~.obj cartoamp~.obj \
	cartodb~.obj cartofreq~.obj cartomag~.obj cartophase~.obj cartopolar~.obj cmplxabs~.obj \
	cmplxadd~.obj cmplxdiv~.obj cmplxmult~.obj cmplxsqrt~.obj cmplxsub~.obj countwrap.obj \
	ctltosig~.obj dbtoamp~.obj dbtomag~.obj degtorad~.obj degtoturn~.obj dspbang~.obj \
	freqsieve~.obj freqtocar~.obj freqtophase~.obj freqtopolar~.obj fundfreq~.obj \
	harmprod~.obj magtoamp~.obj magtodb~.obj magtrim~.obj monitor~.obj oscbank~.obj \
	pafft~.obj paifft~.obj partconv~.obj peaks~.obj phaseaccum~.obj phasedelta~.obj  \
	phasetofreq~.obj piwrap~.obj polartocar~.obj polartofreq~.obj radtodeg~.obj \
	radtoturn~.obj recip~.obj rgbtable.obj rotate~.obj rounder~.obj scale~.obj sigtoctl~.obj \
	softclip~.obj pd_spectral_toolkit.obj tabindex~.obj trunc~.obj \
	turntodeg~.obj turntorad~.obj utility.obj valleys~.obj windower.obj winfft~.obj winifft~.obj \
	nand~.obj neq~.obj nor~.obj not~.obj mod~.obj and~.obj leq~.obj lt~.obj eq~.obj geq~.obj gt~.obj or~.obj


#-------------------------------------------------------------------------------
# Windows Build
#-------------------------------------------------------------------------------
windows: pd_spectral_toolkit.dll

.SUFFIXES: .obj .dll .c

WINDOWSCFLAGS =	\
	/W3 \
	/O2 \
	/DNT \
	/DPD \
	/DPD_LONGINTTYPE="long long" \
	/nologo

WINDOWSINCLUDE = /I. /I\tcl\include /I$(PDSRC) /I$(WINDOWSVC)\include

WINLIBRARY = $(WINDOWSLDIR)\libcmt.lib \
	"C:\Program Files\Pd\bin\pd.lib"

{.\src}.c.obj:
	cl $(WINDOWSCFLAGS) $(WINDOWSINCLUDE) /c $<

pd_spectral_toolkit.dll: $(WINOBJECTS)
	link \
	/dll \
	/out:pd_spectral_toolkit.dll \
	/export:pd_spectral_toolkit_setup \
	$(WINOBJECTS) \
	$(WINLIBRARY)
	del *.obj
	move pd_spectral_toolkit.dll "Pd Spectral Toolkit"


#-------------------------------------------------------------------------------
# MacOS / Linux Compile Command
# makefile trick to have linux, macos make and windows nmake work on same file
#-------------------------------------------------------------------------------

# \
!ifndef 0 # \
!else
ifeq (macos,$(MAKECMDGOALS))
COMPILECOMMAND = $(MACOSCC) $(MACOSCFLAGS) -I$(PDSRC) -c -o $@ $<
endif

ifeq (linux,$(MAKECMDGOALS))
COMPILECOMMAND = cc $(LINUXCFLAGS) -fPIC -I$(PDSRC) -c -o $@ $<
endif

ifeq (linux32,$(MAKECMDGOALS))
COMPILECOMMAND = cc $(LINUXCFLAGS) -I$(PDSRC) -c -o $@ $<
endif
# \
!endif


#-------------------------------------------------------------------------------
# MacOS Build
#-------------------------------------------------------------------------------
macos: pd_spectral_toolkit.pd_darwin

.SUFFIXES: .pd_darwin

MACOSCFLAGS = \
	-DPD \
	-O3 \
	-Wall \
	-W \
	-Wshadow \
	-Wno-unused \
	-Wno-parentheses \
	-Wno-switch \
	-arch $(MACOSARCH) \
	-Wno-maybe-uninitialized \
	-Wno-cast-function-type

%.o: src/%.c
	$(COMPILECOMMAND)

pd_spectral_toolkit.pd_darwin: $(OBJECTS)
	$(MACOSCC) \
	-o pd_spectral_toolkit.pd_darwin \
	-bundle \
	-undefined suppress \
	-arch $(MACOSARCH) \
	-flat_namespace \
	$(OBJECTS)
	rm -f *.o
	mv pd_spectral_toolkit.pd_darwin ./Pd\ Spectral\ Toolkit


#-------------------------------------------------------------------------------
# Linux Build
#-------------------------------------------------------------------------------
linux: pd_spectral_toolkit.l_ia64

linux32: pd_spectral_toolkit.l_i386

.SUFFIXES: .l_i386 .l_ia64

LINUXCFLAGS = \
	-DPD \
	-Ofast \
	-funroll-loops \
	-fomit-frame-pointer \
	-Wall \
	-W \
	-Wshadow \
	-Wno-unused \
	-Wno-parentheses \
	-Wno-switch \
	-Wno-maybe-uninitialized \
	-Wno-cast-function-type \

%.o: src/%.c
	$(COMPILECOMMAND)

pd_spectral_toolkit.l_i386: $(OBJECTS)
	ld \
	-lc \
	-lm \
	-shared \
	-o pd_spectral_toolkit.l_i386 \
	$(OBJECTS)
	strip --strip-unneeded pd_spectral_toolkit.l_i386
	rm -f *.o
	mv pd_spectral_toolkit.l_i386 ./Pd\ Spectral\ Toolkit

pd_spectral_toolkit.l_ia64: $(OBJECTS)
	ld \
	-lc \
	-lm \
	-shared \
	-o pd_spectral_toolkit.l_ia64 \
	$(OBJECTS)
	strip --strip-unneeded pd_spectral_toolkit.l_ia64
	rm -f *.o
	mv pd_spectral_toolkit.l_ia64 ./Pd\ Spectral\ Toolkit


#-------------------------------------------------------------------------------
# Clean
#-------------------------------------------------------------------------------
clean: ; rm -f \
	*.o \
	*.obj \
	*.dll \
	*.l_i386 \
	*.l_ia64 \
	*.pd_darwin \
	./Pd\ Spectral\ Toolkit/*.dll \
	./Pd\ Spectral\ Toolkit/*.l_i386 \
	./Pd\ Spectral\ Toolkit/*.l_ia64 \
	./Pd\ Spectral\ Toolkit/*.pd_darwin


#-------------------------------------------------------------------------------
# Help
#-------------------------------------------------------------------------------
help:
	@echo
	@echo  Pd Spectral Toolkit Makefile
	@echo ------------------------------
	@echo  make macos
	@echo  make linux
	@echo  make linux32
	@echo  make windows
	@echo


#-------------------------------------------------------------------------------
# EOF
#-------------------------------------------------------------------------------
