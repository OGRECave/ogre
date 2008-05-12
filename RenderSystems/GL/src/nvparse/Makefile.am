EXTRA_DIST = ps1.0_grammar.y rc1.0_grammar.y ts1.0_grammar.y vs1.0_grammar.y \
             ps1.0_tokens.l  rc1.0_tokens.l  ts1.0_tokens.l  vs1.0_tokens.l \
             Makefile.reyacc

AM_YFLAGS = -d

INCLUDES = -DYY_NEVER_INTERACTIVE -DYY_NO_UNPUT $(STLPORT_CFLAGS) -I$(top_srcdir)/RenderSystems/GL/include \
           -I$(top_srcdir)/OgreMain/include $(GLSUPPORT_CFLAGS) \
           -I$(top_srcdir)/PlatformManagers/$(OGRE_PLATFORM)/include \
            $(CFLAGS_VISIBILITY)

noinst_LTLIBRARIES = libnvparse.la

libnvparse_la_SOURCES = nvparse.cpp nvparse_errors.cpp rc1.0_combiners.cpp \
                       rc1.0_final.cpp rc1.0_general.cpp ts1.0_inst.cpp \
                       ts1.0_inst_list.cpp _ts1.0_parser.cpp _ts1.0_lexer.cpp \
                       _rc1.0_lexer.cpp _rc1.0_parser.cpp avp1.0_impl.cpp \
 _ps1.0_parser.h _vs1.0_parser.h nvparse_errors.h \
                        rc1.0_combiners.h  rc1.0_register.h _rc1.0_parser.h \
                        macro.h nvparse_externs.h rc1.0_final.h ts1.0_inst.h \
                        vs1.0_inst.h _ts1.0_parser.h nvparse.h \
                        ps1.0_program.h rc1.0_general.h ts1.0_inst_list.h \
                        vs1.0_inst_list.h

if OGRE_LINUX
  libnvparse_la_SOURCES += _vs1.0_lexer.cpp _vs1.0_parser.cpp _ps1.0_lexer.cpp _ps1.0_parser.cpp vs1.0_inst.cpp  vs1.0_inst_list.cpp ps1.0_program.cpp vsp1.0_impl.cpp vcp1.0_impl.cpp vp1.0_impl.cpp 
endif

if OGRE_NT
  INCLUDES += -I$(srcdir)/winheaders
  libnvparse_la_SOURCES += _vs1.0_lexer.cpp _vs1.0_parser.cpp _ps1.0_lexer.cpp _ps1.0_parser.cpp vs1.0_inst.cpp  vs1.0_inst_list.cpp ps1.0_program.cpp vsp1.0_impl.cpp vcp1.0_impl.cpp vp1.0_impl.cpp 
endif

if OGRE_OSX
  INCLUDES += -DMACOS -I../../../../Mac/XCode/OgreFramework
endif

# ifeq ($(OS), "Linux")
#$(MAIN):
#	-bison -o _ps1.0_parser.c ps1.0_grammar.y
#	-mv -f _ps1.0_parser.c _ps1.0_parser.cpp
#	-bison -o _rc1.0_parser.c rc1.0_grammar.y
#	-mv -f _rc1.0_parser.c _rc1.0_parser.cpp
#	-bison -o _ts1.0_parser.c ts1.0_grammar.y
#	-mv -f _ts1.0_parser.c _ts1.0_parser.cpp
#	-bison -o _vs1.0_parser.c vs1.0_grammar.y
#	-mv -f _vs1.0_parser.c _vs1.0_parser.cpp
#	-flex -o_rc1.0_lexer.cpp rc1.0_tokens.l
#	-flex -o_ps1.0_lexer.cpp ps1.0_tokens.l
#	-flex -o_ts1.0_lexer.cpp ts1.0_tokens.l
#	-flex -o_vs1.0_lexer.cpp vs1.0_tokens.l
#	make compile
#endif

#ifeq ($(OS), "Windows_NT")
#$(MAIN):
#	-bison -o _ps1.0_parser.c ps1.0_grammar.y
#	-mv -f _ps1.0_parser.c _ps1.0_parser.cpp
#	-bison -o _rc1.0_parser.c rc1.0_grammar.y
#	-mv -f _rc1.0_parser.c _rc1.0_parser.cpp
#	-bison -o _ts1.0_parser.c ts1.0_grammar.y
#	-mv -f _ts1.0_parser.c _ts1.0_parser.cpp
#	-bison -o _vs1.0_parser.c vs1.0_grammar.y
#	-mv -f _vs1.0_parser.c _vs1.0_parser.cpp
#	-flex -o_rc1.0_lexer.cpp rc1.0_tokens.l
#	-flex -o_ps1.0_lexer.cpp ps1.0_tokens.l
#	-flex -o_ts1.0_lexer.cpp ts1.0_tokens.l
#	-flex -o_vs1.0_lexer.cpp vs1.0_tokens.l
#	make compile
#endif

#ifeq ($(OS), "MacOSX")
#$(MAIN):
#	-bison -o _rc1.0_parser.c rc1.0_grammar.y
#	-mv -f _rc1.0_parser.c _rc1.0_parser.cpp
#	-bison -o _ts1.0_parser.c ts1.0_grammar.y
#	-mv -f _ts1.0_parser.c _ts1.0_parser.cpp
#	-flex -o_rc1.0_lexer.cpp rc1.0_tokens.l
#	-flex -o_ts1.0_lexer.cpp ts1.0_tokens.l
#	make compile
#endif
