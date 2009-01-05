# Configure paths for FreeType2
# Marcelo Magallon 2001-10-26, based on gtk.m4 by Owen Taylor

dnl AC_CHECK_FT2([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for FreeType2, and define FT2_CFLAGS and FT2_LIBS
dnl
AC_DEFUN([AC_CHECK_FT2],
[dnl
dnl Get the cflags and libraries from the freetype-config script
dnl
AC_ARG_WITH(ft-prefix,
[  --with-ft-prefix=PREFIX
                          Prefix where FreeType is installed (optional)],
            ft_config_prefix="$withval", ft_config_prefix="")
AC_ARG_WITH(ft-exec-prefix,
[  --with-ft-exec-prefix=PREFIX
                          Exec prefix where FreeType is installed (optional)],
            ft_config_exec_prefix="$withval", ft_config_exec_prefix="")
AC_ARG_ENABLE(freetypetest,
[  --disable-freetypetest  Do not try to compile and run
                          a test FreeType program],
              [], enable_fttest=yes)

if test x$ft_config_exec_prefix != x ; then
  ft_config_args="$ft_config_args --exec-prefix=$ft_config_exec_prefix"
  if test x${FT2_CONFIG+set} != xset ; then
    FT2_CONFIG=$ft_config_exec_prefix/bin/freetype-config
  fi
fi
if test x$ft_config_prefix != x ; then
  ft_config_args="$ft_config_args --prefix=$ft_config_prefix"
  if test x${FT2_CONFIG+set} != xset ; then
    FT2_CONFIG=$ft_config_prefix/bin/freetype-config
  fi
fi
AC_PATH_PROG(FT2_CONFIG, freetype-config, no)

min_ft_version=ifelse([$1], ,9.1.0,$1)
AC_MSG_CHECKING(for FreeType - version >= $min_ft_version)
no_ft=""
if test "$FT2_CONFIG" = "no" ; then
  no_ft=yes
else
  FT2_CFLAGS=`$FT2_CONFIG $ft_config_args --cflags`
  FT2_LIBS=`$FT2_CONFIG $ft_config_args --libs`
  ft_config_major_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  ft_config_minor_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  ft_config_micro_version=`$FT2_CONFIG $ft_config_args --version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  ft_min_major_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\1/'`
  ft_min_minor_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\2/'`
  ft_min_micro_version=`echo $min_ft_version | \
         sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\)/\3/'`
  if test x$enable_fttest = xyes ; then
    ft_config_is_lt=""
    if test $ft_config_major_version -lt $ft_min_major_version ; then
      ft_config_is_lt=yes
    else
      if test $ft_config_major_version -eq $ft_min_major_version ; then
        if test $ft_config_minor_version -lt $ft_min_minor_version ; then
          ft_config_is_lt=yes
        else
          if test $ft_config_minor_version -eq $ft_min_minor_version ; then
            if test $ft_config_micro_version -lt $ft_min_micro_version ; then
              ft_config_is_lt=yes
            fi
          fi
        fi
      fi
    fi
    if test x$ft_config_is_lt = xyes ; then
      no_ft=yes
    else
      ac_save_CFLAGS="$CFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $FT2_CFLAGS"
      LIBS="$FT2_LIBS $LIBS"
dnl
dnl Sanity checks for the results of freetype-config to some extent
dnl
      AC_TRY_RUN([
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdlib.h>

int
main()
{
  FT_Library library;
  FT_Error error;

  error = FT_Init_FreeType(&library);

  if (error)
    return 1;
  else
  {
    FT_Done_FreeType(library);
    return 0;
  }
}
],, no_ft=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
      CFLAGS="$ac_save_CFLAGS"
      LIBS="$ac_save_LIBS"
    fi             # test $ft_config_version -lt $ft_min_version
  fi               # test x$enable_fttest = xyes
fi                 # test "$FT2_CONFIG" = "no"
if test x$no_ft = x ; then
   AC_MSG_RESULT(yes)
   ifelse([$2], , :, [$2])
else
   AC_MSG_RESULT(no)
   if test "$FT2_CONFIG" = "no" ; then
     echo "*** The freetype-config script installed by FreeType 2 could not be found."
     echo "*** If FreeType 2 was installed in PREFIX, make sure PREFIX/bin is in"
     echo "*** your path, or set the FT2_CONFIG environment variable to the"
     echo "*** full path to freetype-config."
   else
     if test x$ft_config_is_lt = xyes ; then
       echo "*** Your installed version of the FreeType 2 library is too old."
       echo "*** If you have different versions of FreeType 2, make sure that"
       echo "*** correct values for --with-ft-prefix or --with-ft-exec-prefix"
       echo "*** are used, or set the FT2_CONFIG environment variable to the"
       echo "*** full path to freetype-config."
     else
       echo "*** The FreeType test program failed to run.  If your system uses"
       echo "*** shared libraries and they are installed outside the normal"
       echo "*** system library path, make sure the variable LD_LIBRARY_PATH"
       echo "*** (or whatever is appropiate for your system) is correctly set."
     fi
   fi
   FT2_CFLAGS=""
   FT2_LIBS=""
   ifelse([$3], , :, [$3])
fi
AC_SUBST(FT2_CFLAGS)
AC_SUBST(FT2_LIBS)
])

AC_DEFUN([OGRE_USE_STLPORT],
[AC_ARG_WITH(stlport, 
             AC_HELP_STRING([--with-stlport=PATH],
                           [the path to STLPort.]),
             ac_cv_use_stlport=$withval,
             ac_cv_use_stlport=no)
 AC_CACHE_CHECK([whether to use STLPort], ac_cv_use_stlport,
                ac_cv_use_stlport=no)
 if test x$ac_cv_use_stlport != xno; then
     STLPORT_CFLAGS="-I$ac_cv_use_stlport/stlport"
     STLPORT_LIBS="-L$ac_cv_use_stlport/lib -lstlport"
 fi
 AC_SUBST(STLPORT_CFLAGS)
 AC_SUBST(STLPORT_LIBS)
])

AC_DEFUN([OGRE_CHECK_GL],
[AC_ARG_ENABLE(gl,
              AC_HELP_STRING([--enable-gl],
                             [Build the OpenGL Render System]),
              [build_gl=$enableval],
              [build_gl=yes])

AM_CONDITIONAL(BUILD_GLRENDERSYSTEM, test x$build_gl = xyes)

])

AC_DEFUN([OGRE_CHECK_GLES],
[AC_ARG_ENABLE(gles,
              AC_HELP_STRING([--enable-gles],
                             [Build the OpenGL ES Render System]),
              [build_gles=$enableval],
              [build_gles=no])

AM_CONDITIONAL(BUILD_GLESRENDERSYSTEM, test x$build_gles = xyes)

])

AC_DEFUN([OGRE_GET_PLATFORM],
[OGRE_PLATFORM=GLX
 AC_ARG_WITH(platform, 
             AC_HELP_STRING([--with-platform=PLATFORM],
                            [the platform to build, currently GLX or Win32]),
             OGRE_PLATFORM=$withval,
             OGRE_PLATFORM=GLX)

  PLATFORM_CFLAGS=""
  PLATFORM_LIBS=""

  dnl Do the extra checks per type here
  case $OGRE_PLATFORM in 
    GLX)
      AC_CHECK_HEADERS([X11/Intrinsic.h],, [AC_MSG_ERROR("libxt headers not found")])
      AC_CHECK_HEADERS([X11/Xaw/Command.h],, [AC_MSG_ERROR("libxaw headers not found")])
      AC_CHECK_HEADERS([X11/extensions/xf86vmode.h],, [AC_MSG_ERROR("libxf86vm headers not found")],[#include <X11/Xlib.h>])
      AC_CHECK_HEADERS([X11/extensions/Xrandr.h],, [AC_MSG_ERROR("libxrandr headers not found")],[#include <X11/Xlib.h>])
      AC_PATH_X
      if test x"$x_includes" != x; then
        if test x"$x_includes" != xNONE; then
          PLATFORM_CFLAGS="-I$x_includes"
        fi
      fi
      if test x"$x_libraries" != x; then
        if test x"$x_libraries" != xNONE; then
          PLATFORM_LIBS="-L$x_libraries -lX11 -lXaw"
        fi
      dnl In case of xorg 7.x, $x_libraries is empty
      else
        PLATFORM_LIBS="-lX11 -lXaw"
      fi
    ;;
    Win32)
      PLATFORM_CFLAGS=""
      PLATFORM_LIBS="-lgdi32 -lwinmm -ldinput8 -ldxguid"
    ;;
  esac

  AC_SUBST(PLATFORM_CFLAGS)
  AC_SUBST(PLATFORM_LIBS)
  AC_SUBST(OGRE_PLATFORM)
])

AC_DEFUN([OGRE_GET_GLSUPPORT],
[OGRE_GLSUPPORT=none
 AC_ARG_WITH(gl-support, 
             AC_HELP_STRING([--with-gl-support=PLATFORM],
                            [ The GLsupport to build (GLX or Win32). Defaults to the platform. Only set this if you know what you are doing. Use --with-platform otherwise.]),
             OGRE_GLSUPPORT=$withval,
             OGRE_GLSUPPORT=none)

  if test "$OGRE_GLSUPPORT" = "none" ; then
    OGRE_GLSUPPORT="$OGRE_PLATFORM"
    AC_MSG_NOTICE([setting gl-support to platform: $OGRE_GLSUPPORT])
  fi
  if test "$OGRE_GLSUPPORT" = "Win32" ; then
    # Uppercase/lowercase
    OGRE_GLSUPPORT=win32
  fi

  GLSUPPORT_CFLAGS=""
  GLSUPPORT_LIBS=""

  dnl Do the extra checks per type here
  case $OGRE_GLSUPPORT in 
    GLX)
        AC_PATH_X
        if test x"$x_includes" != x; then
          if test x"$x_includes" != xNONE; then
            GLSUPPORT_CFLAGS="-I$x_includes"
          fi
        fi
        if test x"$x_libraries" != x; then
          if test x"$x_libraries" != xNONE; then
            GLSUPPORT_LIBS="-L$x_libraries -lX11 -lXext -lGL -lXrandr -lXxf86vm"
          fi
        dnl In case of xorg 7.x $x_libraries might be empty
        else
          GLSUPPORT_LIBS="-lX11 -lXext -lGL -lXrandr -lXxf86vm"
        fi
    ;;
    win32)
	GLSUPPORT_CFLAGS=""
	GLSUPPORT_LIBS="-lgdi32 -lwinmm"
    ;;
  esac

  AC_SUBST(GLSUPPORT_CFLAGS)
  AC_SUBST(GLSUPPORT_LIBS)
  AC_SUBST(OGRE_GLSUPPORT)
  AC_CONFIG_FILES([RenderSystems/GL/src/GLX/Makefile
                   RenderSystems/GL/src/win32/Makefile])
])

AC_DEFUN([OGRE_GET_GLESSUPPORT],
[OGRE_GLESSUPPORT=none
 AC_ARG_WITH(gles-support,
             AC_HELP_STRING([--with-gles-support=PLATFORM],
                            [ The GLESsupport to build (GLX). Defaults to the platform. Only set this if you know what you are doing. Use --with-platform otherwise.]),
             OGRE_GLESSUPPORT=$withval,
             OGRE_GLESSUPPORT=none)

  if test "$OGRE_GLESSUPPORT" = "none" ; then
    OGRE_GLESSUPPORT=EGL
    AC_MSG_NOTICE([setting gles-support to platform: $OGRE_GLESSUPPORT])
  fi
  if test "$OGRE_GLESSUPPORT" != "EGL" ; then
    AC_MSG_ERROR([setting gles-support to $OGRE_GLESSUPPORT is not supported yet])
  fi

  GLESSUPPORT_CFLAGS=""
  GLESSUPPORT_LIBS=""

  dnl Do the extra checks per type here
  case $OGRE_GLESSUPPORT in
    EGL)
        AC_PATH_X
        if test x"$x_includes" != x; then
          if test x"$x_includes" != xNONE; then
            GLESSUPPORT_CFLAGS="-I$x_includes"
          fi
        fi
        if test x"$x_libraries" != x; then
          if test x"$x_libraries" != xNONE; then
            GLESSUPPORT_LIBS="-L$x_libraries -lX11 -lXext -lGLES_CM -lXrandr -lXxf86vm"
          fi
        dnl In case of xorg 7.x $x_libraries might be empty
        else
          GLESSUPPORT_LIBS="-lX11 -lXext -lGLES_CM -lXrandr -lXxf86vm"
        fi
    ;;
  esac

  AC_SUBST(GLESSUPPORT_CFLAGS)
  AC_SUBST(GLESSUPPORT_LIBS)
  AC_SUBST(OGRE_GLESSUPPORT)
  AC_CONFIG_FILES([RenderSystems/GLES/include/EGL/Makefile
                   RenderSystems/GLES/src/EGL/Makefile])
])


AC_DEFUN([OGRE_SETUP_FOR_TARGET],
[case $host in
*-*-cygwin* | *-*-mingw* | *-*-pw32*)
	AC_SUBST(SHARED_FLAGS, "-shared -no-undefined -Xlinker --export-all-symbols")
	AC_SUBST(PLUGIN_FLAGS, "-shared -no-undefined -avoid-version")
	AC_SUBST(GL_LIBS, "-lopengl32 -lglu32")	
	AC_CHECK_TOOL(RC, windres)
        nt=true
;;
*-*-darwin*)
        AC_SUBST(SHARED_FLAGS, "-shared")
        AC_SUBST(PLUGIN_FLAGS, "-shared -avoid-version")
        AC_SUBST(GL_LIBS, "-lGL -lGLU")
        osx=true
;;
 *) dnl default to standard linux
	AC_SUBST(SHARED_FLAGS, "-shared")
	AC_SUBST(PLUGIN_FLAGS, "-shared -avoid-version")
	AC_SUBST(GL_LIBS, "-lGL -lGLU")
        linux=true
;;
esac
dnl you must arrange for every AM_conditional to run every time configure runs
AM_CONDITIONAL(OGRE_NT, test x$nt = xtrue)
AM_CONDITIONAL(OGRE_LINUX, test x$linux = xtrue)
AM_CONDITIONAL(OGRE_OSX,test x$osx = xtrue )
])


AC_DEFUN([OGRE_DETECT_ENDIAN],
[AC_TRY_RUN([
		int main()
		{
			short s = 1;
			short* ptr = &s;
			unsigned char c = *((char*)ptr);
			return c;
		}
	]
	,[AC_DEFINE(OGRE_CONFIG_BIG_ENDIAN,,[Big endian machine])
          OGRE_CFLAGS="$OGRE_CFLAGS -DOGRE_CONFIG_BIG_ENDIAN"]
	,[AC_DEFINE(OGRE_CONFIG_LITTLE_ENDIAN,,[Little endian machine])
          OGRE_CFLAGS="$OGRE_CFLAGS -DOGRE_CONFIG_LITTLE_ENDIAN"])
])

AC_DEFUN([OGRE_CHECK_OPENEXR],
[AC_ARG_ENABLE(openexr,
              AC_HELP_STRING([--enable-openexr],
                             [Build the OpenEXR plugin]),
              [build_exr=$enableval],
              [build_exr=no])

if test "x$build_exr" = "xyes" ; then
	PKG_CHECK_MODULES(OPENEXR, OpenEXR, [build_exr=yes], [build_exr=no])

	if test "x$build_exr" = "xyes" ; then
	   	AC_CONFIG_FILES([ PlugIns/EXRCodec/Makefile \
    					 PlugIns/EXRCodec/src/Makefile \
    					 PlugIns/EXRCodec/include/Makefile])
		AC_SUBST(OPENEXR_CFLAGS)
		AC_SUBST(OPENEXR_LIBS)

	fi

fi

AM_CONDITIONAL(BUILD_EXRPLUGIN, test x$build_exr = xyes)

])

AC_DEFUN([OGRE_CHECK_CG],
[AC_ARG_ENABLE(cg,
              AC_HELP_STRING([--disable-cg],
                             [Do not build the Cg plugin (recommended you do so!)]),
              [build_cg=$enableval],
              [build_cg=yes])

if test "x$build_cg" = "xyes" ; then
	AC_CHECK_LIB(Cg, cgCreateProgram,,AC_MSG_ERROR([
	****************************************************************
	* You do not have the nVidia Cg libraries installed.           *
	* Go to http://developer.nvidia.com/object/cg_toolkit.html     *
	* (Click on Cg_Linux.tar.gz).                                  *
	* You can disable the building of Cg support by providing      *	
	* --disable-cg to this configure script but this is highly     *
	* discouraged as this breaks many of the examples.             *
	****************************************************************])
	)
fi

AM_CONDITIONAL(BUILD_CGPLUGIN, test x$build_cg = xyes)

])

AC_DEFUN([OGRE_CHECK_CPPUNIT],
[
AM_PATH_CPPUNIT([1.10.0], [build_unit_tests=true])
AM_CONDITIONAL([BUILD_UNIT_TESTS], [test x$build_unit_tests = xtrue])
])


AC_DEFUN([OGRE_CHECK_DX9],
[AC_ARG_ENABLE(direct3d,
              AC_HELP_STRING([--enable-direct3d],
                             [Build the DirectX 9 Render System]),
              [build_dx9=$enableval],
              [build_dx9=no])

AM_CONDITIONAL(BUILD_DX9RENDERSYSTEM, test x$build_dx9 = xyes)

])


AC_DEFUN([OGRE_CHECK_FREEIMAGE],
[AC_ARG_ENABLE(freeimage,
              AC_HELP_STRING([--disable-freeimage],
                             [Don't use FreeImage for image loading. This is not recommended unless you provide your own image loading codecs.]),
              [build_freeimage=$enableval],
              [build_freeimage=yes])


AM_CONDITIONAL(USE_FREEIMAGE, test x$build_freeimage = xyes)

if test "x$build_freeimage" = "xyes" ; then
	AC_CHECK_LIB(freeimage, FreeImage_Load,,AC_MSG_ERROR([
****************************************************************
* You do not have FreeImage installed.  This is required.      *
* You may find it at http://freeimage.sourceforge.net/.        *
* Note: You can also provide --disable-freeimage to the build  *
* process to build without it. This is an advanced option      *
* useful only if you provide your own image loading codecs.    *
****************************************************************]), -lstdc++)
	AC_DEFINE([OGRE_NO_FREEIMAGE], [0], [Do not use freeimage to load images])
else
	AC_DEFINE([OGRE_NO_FREEIMAGE], [1], [Load images using the freeimage library])
	OGRE_CFLAGS="$OGRE_CFLAGS -DOGRE_NO_FREEIMAGE"
fi


])


AC_DEFUN([OGRE_CHECK_DEVIL],
[AC_ARG_ENABLE(devil,
              AC_HELP_STRING([--disable-devil],
                             [Don't use DevIL for image loading. This is not recommended unless you provide your own image loading codecs.]),
              [build_il=$enableval],
              [build_il=yes])

AM_CONDITIONAL(USE_DEVIL, test x$build_il = xyes && test x$build_freeimage = xno)

if test "x$build_freeimage" = "xyes"; then
    AC_MSG_NOTICE([Freeimage is being built, disabling check for DevIL.])
    [build_il=no]
	AC_DEFINE([OGRE_NO_DEVIL], [1], [Build devil])
else
if test "x$build_il" = "xyes"; then
	AC_CHECK_LIB(IL, ilInit,,AC_MSG_ERROR([
****************************************************************
* You do not have DevIL installed.  This is required to build. *
* You may find it at http://openil.sourceforge.net/.           *
* Note: You can also provide --disable-devil to the build      *
* process to build without DevIL. This is an advanced option   *
* useful only if you provide your own image loading codecs.    *
****************************************************************]))
	AC_CHECK_LIB(ILU, iluFlipImage)
	AC_DEFINE([OGRE_NO_DEVIL], [0], [Build devil])
else
	AC_DEFINE([OGRE_NO_DEVIL], [1], [Build devil])
fi
fi

])



AC_DEFUN([OGRE_CHECK_PIC],
[
AC_MSG_CHECKING([whether -fPIC is needed])
    case $host in
        x86_64-*)
            CXXFLAGS="$CXXFLAGS -fPIC"
            AC_MSG_RESULT(yes)
        ;;
        *)
            AC_MSG_RESULT(no)
        ;;
    esac
])

AC_DEFUN([OGRE_CHECK_CEGUI], [
    PKG_CHECK_MODULES(CEGUI, CEGUI >= 0.5.0, 
            [build_cegui_sample=true], [build_cegui_sample=false])
    if test x$build_cegui_sample = xtrue; then
        AC_CONFIG_FILES([Samples/Common/CEGUIRenderer/Makefile \
                         Samples/Common/CEGUIRenderer/CEGUI-OGRE.pc
                         Samples/Common/CEGUIRenderer/src/Makefile \
                         Samples/Common/CEGUIRenderer/include/Makefile \
                         Samples/Gui/Makefile \
                         Samples/Gui/src/Makefile])
        AC_SUBST(CEGUI_CFLAGS)
        AC_SUBST(CEGUI_LIBS)
        AC_MSG_RESULT([CEGUI available, Gui and FacialAnimation samples will be built])
    else
        AC_MSG_RESULT([CEGUI not available, Gui and FacialAnimation samples will not be built])
    fi
    AM_CONDITIONAL([HAVE_CEGUI], [test x$build_cegui_sample = xtrue])
])

AC_DEFUN([OGRE_CHECK_DOUBLE],
[
AC_ARG_ENABLE(double,
              AC_HELP_STRING([--enable-double],
                             [Build OGRE in double floating point precision mode. This is not recommended for normal use as it is slower.]),
              [build_double=$enableval],
              [build_double=no])
AC_MSG_CHECKING([whether to use double floating point precision])
	case $build_double in
        yes)
			AC_DEFINE([OGRE_DOUBLE_PRECISION], [1], [Build with double precision])
			OGRE_CFLAGS="$OGRE_CFLAGS -DOGRE_DOUBLE_PRECISION"
			AC_MSG_RESULT(yes)
        ;;
        *)
			AC_DEFINE([OGRE_DOUBLE_PRECISION], [0], [Build with single precision])
			AC_MSG_RESULT(no)
        ;;
    esac
])

#   Copyright (c) 2007 Thomas Porschberg <thomas@randspringer.de>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.
AC_DEFUN([AX_BOOST],
[
    AC_ARG_WITH([boost],
                AS_HELP_STRING([--with-boost=DIR],
                [use boost (default is yes) specify the root directory for boost library (optional)]),
                [
                if test "$withval" = "no"; then
		            want_boost="no"
                elif test "$withval" = "yes"; then
                    want_boost="yes"
                    ac_boost_path=""
                else
			        want_boost="yes"
            		ac_boost_path="$withval"
		        fi
            	],
                [want_boost="yes"])

    AC_CANONICAL_BUILD
	if test "x$want_boost" = "xyes"; then
        AC_REQUIRE([AC_PROG_CC])
		boost_lib_version_req=ifelse([$1], ,1.31.0,$1)
		boost_lib_version_req_shorten=`expr $boost_lib_version_req : '\([[0-9]]*\.[[0-9]]*\)'`
		boost_lib_version_req_major=`expr $boost_lib_version_req : '\([[0-9]]*\)'`
		boost_lib_version_req_minor=`expr $boost_lib_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
		boost_lib_version_req_sub_minor=`expr $boost_lib_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
		if test "x$boost_lib_version_req_sub_minor" = "x" ; then
			boost_lib_version_req_sub_minor="0"
    	fi
		WANT_BOOST_VERSION=`expr $boost_lib_version_req_major \* 100000 \+  $boost_lib_version_req_minor \* 100 \+ $boost_lib_version_req_sub_minor`
		AC_MSG_CHECKING(for boostlib >= $boost_lib_version_req)
		succeeded=no

		dnl first we check the system location for boost libraries
		dnl this location ist chosen if boost libraries are installed with the --layout=system option
		dnl or if you install boost with RPM
		if test "$ac_boost_path" != ""; then
			BOOST_LDFLAGS="-L$ac_boost_path/lib"
			BOOST_CPPFLAGS="-I$ac_boost_path/include"
		else
			for ac_boost_path_tmp in /usr /usr/local /opt ; do
				if test -d "$ac_boost_path_tmp/include/boost" && test -r "$ac_boost_path_tmp/include/boost"; then
					BOOST_LDFLAGS="-L$ac_boost_path_tmp/lib"
					BOOST_CPPFLAGS="-I$ac_boost_path_tmp/include"
					break;
				fi
			done
		fi

		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
		export LDFLAGS

	AC_LANG_PUSH(C++)
     	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
@%:@include <boost/version.hpp>
]],
       [[
#if BOOST_VERSION >= $WANT_BOOST_VERSION
// Everything is okay
#else
#  error Boost version is too old
#endif

		]])],
    	[
         AC_MSG_RESULT(yes)
		 succeeded=yes
		 found_system=yes
         ifelse([$2], , :, [$2])
       ],
       [
       ])
       AC_LANG_POP([C++])
		dnl if we found no boost with system layout we search for boost libraries
		dnl built and installed without the --layout=system option or for a staged(not installed) version
		if test "x$succeeded" != "xyes"; then
			_version=0
			if test "$ac_boost_path" != ""; then
                BOOST_LDFLAGS="-L$ac_boost_path/lib"
				if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
					for i in `ls -d $ac_boost_path/include/boost-* 2>/dev/null`; do
						_version_tmp=`echo $i | sed "s#$ac_boost_path##" | sed 's/\/include\/boost-//' | sed 's/_/./'`
						V_CHECK=`expr $_version_tmp \> $_version`
						if test "$V_CHECK" = "1" ; then
							_version=$_version_tmp
						fi
						VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
						BOOST_CPPFLAGS="-I$ac_boost_path/include/boost-$VERSION_UNDERSCORE"
					done
				fi
			else
				for ac_boost_path in /usr /usr/local /opt ; do
					if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
						for i in `ls -d $ac_boost_path/include/boost-* 2>/dev/null`; do
							_version_tmp=`echo $i | sed "s#$ac_boost_path##" | sed 's/\/include\/boost-//' | sed 's/_/./'`
							V_CHECK=`expr $_version_tmp \> $_version`
							if test "$V_CHECK" = "1" ; then
								_version=$_version_tmp
								best_path=$ac_boost_path
							fi
						done
					fi
				done

				VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
				BOOST_CPPFLAGS="-I$best_path/include/boost-$VERSION_UNDERSCORE"
				BOOST_LDFLAGS="-L$best_path/lib"

	    		if test "x$BOOST_ROOT" != "x"; then
                    if test -d "$BOOST_ROOT" && test -r "$BOOST_ROOT" && test -d "$BOOST_ROOT/stage/lib" && test -r "$BOOST_ROOT/stage/lib"; then
						version_dir=`expr //$BOOST_ROOT : '.*/\(.*\)'`
						stage_version=`echo $version_dir | sed 's/boost_//' | sed 's/_/./g'`
						stage_version_shorten=`expr $stage_version : '\([[0-9]]*\.[[0-9]]*\)'`
						V_CHECK=`expr $stage_version_shorten \>\= $_version`
						if test "$V_CHECK" = "1" ; then
							AC_MSG_NOTICE(We will use a staged boost library from $BOOST_ROOT)
							BOOST_CPPFLAGS="-I$BOOST_ROOT"
							BOOST_LDFLAGS="-L$BOOST_ROOT/stage/lib"
						fi
					fi
	    		fi
			fi

			CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
			export CPPFLAGS
			LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
			export LDFLAGS

            AC_LANG_PUSH(C++)
            AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
@%:@include <boost/version.hpp>
]],
       [[
#if BOOST_VERSION >= $WANT_BOOST_VERSION
// Everything is okay
#else
#  error Boost version is too old
#endif

		]])],
    	[
         AC_MSG_RESULT(yes ($_version))
		 succeeded=yes
         ifelse([$2], , :, [$2])
       ],
       [
         AC_MSG_RESULT(no ($_version))
         ifelse([$3], , :, [$3])
       ])
    	AC_LANG_POP([C++])
		fi

		if test "$succeeded" != "yes" ; then
			if test "$_version" = "0" ; then
				AC_MSG_ERROR([[We could not detect the boost libraries (version $boost_lib_version_req_shorten or higher). If you have a staged boost library (still not installed) please specify \$BOOST_ROOT in your environment and do not give a PATH to --with-boost option.  If you are sure you have boost installed, then check your version number looking in <boost/version.hpp>. See http://randspringer.de/boost for more documentation.]])
			else
				AC_MSG_ERROR('Your boost libraries seems to old (version $_version).  We need at least $boost_lib_version_shorten')
			fi
		else
			AC_SUBST(BOOST_CPPFLAGS)
			AC_SUBST(BOOST_LDFLAGS)
			AC_DEFINE(HAVE_BOOST,,[define if the Boost library is available])


			AC_CACHE_CHECK(whether the Boost::Thread library is available,
						   ax_cv_boost_thread,
						[AC_LANG_PUSH([C++])
			 CXXFLAGS_SAVE=$CXXFLAGS

			 if test "x$build_os" = "xsolaris" ; then
  				 CXXFLAGS="-pthreads $CXXFLAGS"
			 elif test "x$build_os" = "xming32" ; then
				 CXXFLAGS="-mthreads $CXXFLAGS"
			 else
				CXXFLAGS="-pthread $CXXFLAGS"
			 fi
			 AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[@%:@include <boost/thread/thread.hpp>]],
                                   [[boost::thread_group thrds;
                                   return 0;]]),
                   ax_cv_boost_thread=yes, ax_cv_boost_thread=no)
			 CXXFLAGS=$CXXFLAGS_SAVE
             AC_LANG_POP([C++])
			])
			if test "x$ax_cv_boost_thread" = "xyes"; then
               if test "x$build_os" = "xsolaris" ; then
 				  BOOST_CPPFLAGS="-pthreads $BOOST_CPPFLAGS"
			   elif test "x$build_os" = "xming32" ; then
 				  BOOST_CPPFLAGS="-mthreads $BOOST_CPPFLAGS"
			   else
				  BOOST_CPPFLAGS="-pthread $BOOST_CPPFLAGS"
			   fi

				AC_SUBST(BOOST_CPPFLAGS)
				AC_DEFINE(HAVE_BOOST_THREAD,,[define if the Boost::THREAD library is available])
				BN=boost_thread
     			LDFLAGS_SAVE=$LDFLAGS
                        case "x$build_os" in
                          *bsd* )
                               LDFLAGS="-pthread $LDFLAGS"
                          break;
                          ;;
                        esac
                echo $CC
                TMPCC=$CC`$CC -dumpversion |sed  -e s/@<:@^a-zA-Z0-9@:>@//g|sed -e s/.$//g`
                echo $TMPCC
				for ax_lib in $BN-$TMPCC $BN-$TMPCC-mt $BN-$TMPCC-mt-s $BN-$TMPCC-s $BN-$CC $BN-$CC-mt $BN-$CC-mt-s $BN-$CC-s $BN $BN-mt \
                              lib$BN-$TMPCC lib$BN-$TMPCC-mt lib$BN-$TMPCC-mt-s lib$BN-$TMPCC-s lib$BN-$CC lib$BN-$CC-mt lib$BN-$CC-mt-s lib$BN-$CC-s lib$BN lib$BN-mt \
                              $BN-mgw $BN-mgw $BN-mgw-mt $BN-mgw-mt-s $BN-mgw-s ; do
				    AC_CHECK_LIB($ax_lib, main, [BOOST_THREAD_LIB="-l$ax_lib"; AC_SUBST(BOOST_THREAD_LIB) link_thread="yes"; break],
                                 [link_thread="no"])
  				done
				if test "x$link_thread" = "xno"; then
					AC_MSG_NOTICE(Could not link against $ax_lib !)
                else
                    case "x$build_os" in
                       *bsd* )
                       BOOST_LDFLAGS="-pthread $BOOST_LDFLAGS"
                       break;
                       ;;
                    esac
				fi
			fi
		fi
        CPPFLAGS="$CPPFLAGS_SAVED"
        LDFLAGS="$LDFLAGS_SAVED"
	fi
])

AC_DEFUN([OGRE_CHECK_ALLOCATOR],
[
    AC_ARG_WITH([allocator],
        AC_HELP_STRING([--with-allocator=type], [Select the memory allocator type to use (std, ned) (default: ned)]),
        [with_allocator=${withval}], [with_allocator=ned])
    if test "x$with_allocator" == "xstd"; then
        AC_DEFINE([OGRE_MEMORY_ALLOCATOR], [1], [Custom memory allocator setting])
    fi
    if test "x$with_allocator" == "xned"; then
        AC_DEFINE([OGRE_MEMORY_ALLOCATOR], [2], [Custom memory allocator setting])
    fi
])

AC_DEFUN([OGRE_CHECK_THREADING],
[
AC_ARG_ENABLE(threading,
              [--enable-threading[=type]  Indicate general support for multithreading. This will
                         enable threading support in certain parts of the engine
                         mainly resource loading and SharedPtr handling. 
                         WARNING: highly experimental, use with caution.
                          full: Another thready may call the graphics lib
                          semi: Calls to graphics lib come from the main thread
                          no @<:@default@:>@: the application uses a single thread]
                             ,
              [build_threads=$enableval],
              [build_threads=no])
    case $build_threads in
        no)
            AC_DEFINE([OGRE_THREAD_SUPPORT], [0], [Build with thread support])
            OGRE_THREAD_LIBS=""
            AC_MSG_RESULT(no)
        ;;
        *)
        AC_REQUIRE([AC_PROG_CC])
        AC_REQUIRE([AC_CANONICAL_BUILD])
        AX_BOOST
		CPPFLAGS_SAVED="$CPPFLAGS"
		CPPFLAGS="$CPPFLAGS $BOOST_CPPFLAGS"
		export CPPFLAGS

		LDFLAGS_SAVED="$LDFLAGS"
		LDFLAGS="$LDFLAGS $BOOST_LDFLAGS"
		export LDFLAGS

        AC_CACHE_CHECK(whether the Boost::Thread library is available,
					   ax_cv_boost_thread,
        [AC_LANG_PUSH([C++])
			 CXXFLAGS_SAVE=$CXXFLAGS

			 if test "x$build_os" = "xsolaris" ; then
  				 CXXFLAGS="-pthreads $CXXFLAGS"
			 elif test "x$build_os" = "xming32" ; then
				 CXXFLAGS="-mthreads $CXXFLAGS"
			 else
				CXXFLAGS="-pthread $CXXFLAGS"
			 fi
			 AC_COMPILE_IFELSE(AC_LANG_PROGRAM([[@%:@include <boost/thread/thread.hpp>]],
                                   [[boost::thread_group thrds;
                                   return 0;]]),
                   ax_cv_boost_thread=yes, ax_cv_boost_thread=no)
			 CXXFLAGS=$CXXFLAGS_SAVE
             AC_LANG_POP([C++])
		])
		if test "x$ax_cv_boost_thread" = "xyes"; then
           if test "x$build_os" = "xsolaris" ; then
			  BOOST_CPPFLAGS="-pthreads $BOOST_CPPFLAGS"
		   elif test "x$build_os" = "xming32" ; then
			  BOOST_CPPFLAGS="-mthreads $BOOST_CPPFLAGS"
		   else
			  BOOST_CPPFLAGS="-pthread $BOOST_CPPFLAGS"
		   fi

			AC_SUBST(BOOST_CPPFLAGS)

			AC_DEFINE(HAVE_BOOST_THREAD,,[define if the Boost::Thread library is available])
			BN=boost_thread

			LDFLAGS_SAVE=$LDFLAGS
                        case "x$build_os" in
                          *bsd* )
                               LDFLAGS="-pthread $LDFLAGS"
                          break;
                          ;;
                        esac
            TMPCC=$CC`$CC -dumpversion |sed  -e s/@<:@^a-zA-Z0-9@:>@//g|sed -e s/.$//g`
            if test "x$ax_boost_user_thread_lib" = "x"; then
				for ax_lib in $BN-$TMPCC $BN-$TMPCC-mt $BN-$TMPCC-mt-s $BN-$TMPCC-s $BN-$CC $BN-$CC-mt $BN-$CC-mt-s $BN-$CC-s $BN $BN-mt \
                              lib$BN-$TMPCC lib$BN-$TMPCC-mt lib$BN-$TMPCC-mt-s lib$BN-$TMPCC-s lib$BN-$CC lib$BN-$CC-mt lib$BN-$CC-mt-s lib$BN-$CC-s lib$BN lib$BN-mt \
                              $BN-mgw $BN-mgw $BN-mgw-mt $BN-mgw-mt-s $BN-mgw-s ; do
				    AC_CHECK_LIB($ax_lib, main, [BOOST_THREAD_LIB="-l$ax_lib"; AC_SUBST(BOOST_THREAD_LIB) link_thread="yes"; break],
                                 [link_thread="no"])
  				done
            else
               for ax_lib in $ax_boost_user_thread_lib $BN-$ax_boost_user_thread_lib; do
				      AC_CHECK_LIB($ax_lib, main,
                                   [BOOST_THREAD_LIB="-l$ax_lib"; AC_SUBST(BOOST_THREAD_LIB) link_thread="yes"; break],
                                   [link_thread="no"])
                  done

            fi
			if test "x$link_thread" = "xno"; then
				AC_MSG_ERROR(Could not link against $ax_lib !)
                        else
                           case "x$build_os" in
                              *bsd* )
			        BOOST_LDFLAGS="-pthread $BOOST_LDFLAGS"
                              break;
                              ;;
                           esac

			fi
		fi
		CPPFLAGS="$CPPFLAGS_SAVED"
    	LDFLAGS="$LDFLAGS_SAVED"
        PLATFORM_CFLAGS="$PLATFORM_CFLAGS $BOOST_CPPFLAGS";
        STLPORT_CFLAGS="$STLPORT_CFLAGS $BOOST_CPPFLAGS";
        #STLPORT BECAUSE THE LIBRARIES DONT HAVE ANY WAY TO GET C++ FLAGS PASSED INTO THEM SO WE CAN TELL THEM WHERE BOOST LIVES
        AC_SUBST(STLPORT_CFLAGS)
        case $build_threads in 
           full)
             thread_flags=1
             AC_DEFINE([OGRE_THREAD_SUPPORT], [1], [Build with thread support])
             OGRE_CFLAGS="$OGRE_CFLAGS -DOGRE_THREAD_SUPPORT=1 $BOOST_CPPFLAGS"
             ;;
           *)
             thread_flags=2
             AC_DEFINE([OGRE_THREAD_SUPPORT], [2], [Build with thread support])             
             OGRE_CFLAGS="$OGRE_CFLAGS -DOGRE_THREAD_SUPPORT=2 $BOOST_CPPFLAGS"
             build_threads=[semi]
             ;;
         esac

         # Check for the C++ Boost library
         OGRE_THREAD_LIBS="$BOOST_LDFLAGS $BOOST_THREAD_LIB";
        ;;
    esac

    AC_SUBST(PLATFORM_CFLAGS)
    AC_SUBST(OGRE_THREAD_LIBS)
])

AC_DEFUN([OGRE_BUILD_DEMOS], [
    AC_ARG_ENABLE(ogre_demos,
        AC_HELP_STRING([--disable-ogre-demos], 
            [Do not build Ogre demos (CEGUI Renderer - if enabled/disabled, is not affected by this]),
            [build_ogre_demos=$enableval], [build_ogre_demos=yes])

    if test "x$build_ogre_demos" = "xyes" ; then
        PKG_CHECK_MODULES(OIS, OIS >= 1.0.0, [ois_found=yes],[ois_found=no])
        AC_SUBST(OIS_CFLAGS)
        AC_SUBST(OIS_LIBS)

        if test "x$ois_found" = "xyes" ; then
           AC_MSG_NOTICE([*** Ogre Demos will be built ***])
        else
          build_ogre_demos=no
          AC_MSG_NOTICE([
****************************************************************
* You do not have OIS installed.  This is required to build    *
* Ogre demos. You may find it at:                              *
* http://www.sourceforge.net/projects/wgois.                   *
* If you do not want to build the demos, you can safely ignore *
* this.                                                        *
****************************************************************])
        fi
    else
        build_ogre_demos=no
        AC_MSG_NOTICE([*** Building of Ogre demos disabled ***])
    fi

    AM_CONDITIONAL([OGRE_BUILDING_DEMOS], [test x$build_ogre_demos = xyes])
])

dnl GUI selection support for configuration/error dialogs
AC_DEFUN([OGRE_CHECK_GUI],
[
    AC_ARG_WITH([gui],
        AC_HELP_STRING([--with-gui=type], [Select the GUI type to use for dialogs (win32, gtk, Xt) (default: auto)]),
        [with_gui=${withval}], [with_gui=auto])

    #remove any old files
    rm -f OgreMain/src/OgreConfigDialog.lo OgreMain/src/OgreErrorDialog.lo

    # Prefer win32, then Xt. gtk is being phased out and must be explicitly specified.
    if test "x$with_gui" == "xauto" && test "x$OGRE_PLATFORM" == "xWIN32"; then
        with_gui=win32
    fi

    if test "x$with_gui" == "xgtk"; then
        PKG_CHECK_MODULES(GTK, gtk+-2.0 >= 2.0.0, [with_gui=gtk], [
            AC_MSG_ERROR([You chose gtk for the GUI but gtk is not available.])
        ])
    fi

    if test "x$with_gui" == "xauto"; then
        with_gui=Xt
    fi

    if test "x$with_gui" == "xwin32"; then
        OGRE_GUI=WIN32
    elif test "x$with_gui" == "xgtk"; then
        OGRE_GUI=gtk
    elif test "x$with_gui" == "xXt"; then
        OGRE_GUI=GLX
        PLATFORM_LIBS="$PLATFORM_LIBS -lXt -lSM -lICE"
    else
        AC_MSG_ERROR([The GUI dialogs for $with_gui are not available.])
    fi

    # Add the OGRE_GUI_xxx flag to compiler command line
    PLATFORM_CFLAGS="$PLATFORM_CFLAGS -DOGRE_GUI_$OGRE_GUI"
    # And export it to client applications as well
    OGRE_CFLAGS="$OGRE_CFLAGS -DOGRE_GUI_$OGRE_GUI"

    AC_SUBST(OGRE_GUI)
    AC_SUBST(GTK_CFLAGS)
    AC_SUBST(GTK_LIBS)
])

dnl Check whether Ogre platform and GUI to be built are the same
dnl Fixes problem with running make distclean
AC_DEFUN([OGRE_PLATFORM_AND_GUI],
[AM_CONDITIONAL(SAME_PLATFORM_AND_GUI, test "x$OGRE_PLATFORM" = "x$OGRE_GUI")
])

dnl SSE support 
AC_DEFUN([OGRE_CHECK_SSE],
[
AC_MSG_CHECKING(whether to use SSE)
case $target_cpu in
	i386 | i486 | i586 | i686 | x86_64 | amd64)
		build_sse=yes
	;;
	*)
		build_sse=no
	;;
esac
AC_MSG_RESULT($build_sse)
AM_CONDITIONAL(OGRE_BUILD_SSE, test x$build_sse = xyes)
])
