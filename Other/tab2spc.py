#! /usr/bin/env python

import sys
import os
import getopt
import string
import re

def fnParse( arg, dirname, files ):
    for file in files:
        for ext in arg:
            if file.rfind( ext, len(file)-len(ext), len(file) ) != -1 and not os.path.isdir( dirname + "/" + file ):
                fo = open( dirname + "/" + file, "rb" )
                lines = fo.read()
                if not '\0' in lines:
                    newlines = re.sub( "\t", globals()['spaces'], lines )
                    if newlines != lines:
                        backup = open( dirname + "/" + file + "~", "wb" )
                        backup.write( lines )
                        backup.close()

                        fo.close()
                        fo = open( dirname + "/" + file, "wb" )
                        fo.write(newlines)

                        print "File " + dirname + "/" + file + " was changed."
                fo.close()

def fnHelp():
    helpstr = """
This program transforms all the TAB characters in all
the files with the given extension from the directory
tree into the given number of spaces.

Syntax:
    tab2spc [num_spaces] [ext1] [ext2] [...]

Extensions are given in the form:
    .cpp .h .txt (no asterisk at the beginning)

Copyright (c) 2002 by Adrian Cearnau (cearny@cearny.ro)
Any use, commercial or not, is allowed
"""
    print helpstr

if len(sys.argv) != 3:
    fnHelp()
else:
    spaces = ""
    for i in range( 0, int(sys.argv[1]) ):
        spaces = spaces + ' '
    exts = sys.argv[2].split()
    currdir = os.getcwd()
    os.path.walk( currdir, fnParse, exts )
    #print "\nTotally", num_files, "files were patched\n"
    print "done"