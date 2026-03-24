#!/bin/sh

# this is a script to compile the cppVersionTesting on all supported c++ versions
# assuming your compiler is g++ or sufficiently similar

if [ "$1" = --help ]; then 
	echo "USAGE './testVersions.sh' <path-to-built-files> "
	echo "CWD must be <ProjectRoot>/CppVersionTesting"
	exit 0
fi 

if test ! -d "$1/include"; then 
	echo "you must pass the path to the build file as the first argument"
	exit 1
fi

includes='-I../OgreMain/include/ -I'"$1"'/include'

for version in 14 17 20 23; do
	c++ --std=c++$version $includes -fsyntax-only *.cpp 
	if test $? == 0; then
		echo "compiling c++$version succeeded"
	else
		echo "compiling c++$version failed"
	fi;
done;
