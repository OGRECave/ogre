#!/bin/bash

# define error codes
E_NOARGS=65
E_BADARG=66

# determine the project file extensions to use for each SDK 
SDKTYPES="[VC7, VC8, VC9, CBMINGW, CBMINGW_STLP]"
case "$1" in
# if no parameter was passed then exit with usage message
"") echo "Usage: `basename $0` $SDKTYPES"; exit $E_NOARGS ;;

"CBMINGW") PROJEXT=".cbp" ; RMEXT="_stlp.cbp samples/scripts/*_linux.cbp" ;;

"CBMINGW_STLP") PROJEXT="_stlp.cbp" ; RMEXT= ;;

"VC7") PROJEXT=".vcproj" ; RMEXT="_vc8.vcproj" ;;

"VC8") PROJEXT="_vc8.vcproj" ; RMEXT= ;;

"VC9") PROJEXT="_vc9.vcproj" ; RMEXT= ;;

# parameter is not valid so exit with usage message
*) echo "SDK: $1 not understood."
   echo  "Usage: `basename $0` $SDKTYPES"; exit $E_BADARG ;;

esac

echo "copying and processing sample scripts for SDK: $1"

rm -R samples/refapp
rm -R samples/scripts
rm -R samples/src
rm -R samples/include
mkdir samples/scripts
mkdir samples/refapp
mkdir samples/refapp/scripts
mkdir samples/src
mkdir samples/include

# process the project files but only do the ones required for a specific SDK
/bin/find ../../samples -iname *$PROJEXT -exec cp \{\} samples/scripts \;
if [[ "$1" == "VC8" ]] 
then
	/bin/find ../../samples -iname *_vc8.vcproj.user -exec cp \{\} samples/scripts \;
	cp ../../ReferenceApplication/BspCollision/scripts/*_vc8.vcproj.user samples/scripts
	/bin/find samples/scripts/ -iname *_vc8.vcproj.user -exec sed -i -f altersamples.sed \{\} \;
fi
if [[ "$1" == "VC9" ]] 
then
	/bin/find ../../samples -iname *_vc9.vcproj.user -exec cp \{\} samples/scripts \;
	cp ../../ReferenceApplication/BspCollision/scripts/*_vc9.vcproj.user samples/scripts
	/bin/find samples/scripts/ -iname *_vc9.vcproj.user -exec sed -i -f altersamples.sed \{\} \;
fi
cp ../../ReferenceApplication/BspCollision/scripts/*$PROJEXT samples/scripts
cp ../../ReferenceApplication/ReferenceAppLayer/scripts/*$PROJEXT samples/refapp/scripts

# only proces file deletions if RMEXT was set
if [ -n "$RMEXT" ]
then
 # remove unwanted scripts that got copied over
 rm samples/scripts/*$RMEXT
 rm samples/refapp/scripts/*$RMEXT
fi
rm samples/scripts/OgreGUIRenderer$PROJEXT
/bin/find samples/scripts/ -iname *$PROJEXT -exec sed -i -f altersamples.sed \{\} \;
/bin/find samples/refapp/scripts/ -iname *$PROJEXT -exec sed -i -f alterrefapp.sed \{\} \;

# Combine the include / src folders; easier to do here than in setup
/bin/find ../../samples -iname *.cpp -exec cp \{\} samples/src \;
/bin/find ../../samples -iname *.h -exec cp \{\} samples/include \;
cp ../../ReferenceApplication/BspCollision/src/*.cpp samples/src

# Copy and alter resources.cfg
cp ../../Samples/Common/bin/Release/resources.cfg samples/
sed -i -e 's/\.\.\/\.\.\/\.\.\/Media/..\/..\/media/i' samples/resources.cfg
