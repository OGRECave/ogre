#!/bin/bash
# ----------------------------------------------------------------------------
# OGRE Documentation Generation Script
# 
# This script generates the manuals and APIs from source files in this folder
# To run this script, you require:
#   - texi2html
# Run from the Docs folder. For example:
# ./src/makedocs.sh
# ----------------------------------------------------------------------------

# Remove old manual
rm -rf manual vbo-update 

# Generate manuals from texi
for f in src/*.texi;
do
	texi2html -Verbose --css-include=style.css --output=`basename $f .texi` -split=node -top_file=index.html $f
done

# Copy stylesheet to core docs folder
cp src/style.css .
	
# Copy images to the manual folder
mkdir -p manual/images
cp src/images/* manual/images/
