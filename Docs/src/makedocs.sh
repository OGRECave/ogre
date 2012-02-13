#!/bin/bash
# ----------------------------------------------------------------------------
# OGRE Documentation Generation Script
# 
# This script generates the manuals and APIs from source files in this folder
# To run this script, you require:
#   1. Doxygen
#   2. texi2html
# Run from the Docs folder. For example:
# ./src/makedocs.sh
# ----------------------------------------------------------------------------

# Generate API docs using doxygen
doxygen src/html.cfg

# Generate manuals from texi
for f in src/*.texi;
do
	texi2html -Verbose --css-include=style.css --output=`basename $f .texi` -split=node -top_file=index.html $f
done
# copy stylesheet to core docs folder
cp src/style.css .
	
