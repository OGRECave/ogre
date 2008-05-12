#!/bin/bash
# ----------------------------------------------------------------------------
# OGRE Documentation Generation Script
# 
# This script generates the manuals and APIs from source files in this folder
# To run this script, you require:
#   1. Doxygen
#   2. texi2html
# ----------------------------------------------------------------------------

# Generate API docs using doxygen
doxygen html.cfg

# Generate manuals from texi
for f in *.texi;
do
	texi2html -Verbose -init_file ogretexi2html.init -subdir=../`basename $f .texi` -split=section -top_file=index.html $f
done
# copy stylesheet to core docs folder
cp style.css ../
	
