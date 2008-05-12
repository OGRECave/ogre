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
                print "Parsing file " + dirname + "/" + file + "..."
                fo = open( dirname + "/" + file, "r" )
                lines = fo.readlines()
                fo.close()
                fo = open( dirname + "/" + file, "r" )
                newlines = fo.readlines()
                changed = 0
                for i in range(0, len(newlines) - 1):
                    newlines[i] = re.sub( "[ \t]+$", "", newlines[i] )
                    changed = 1
                if changed == 1:
                    backup = open( dirname + "/" + file + "~", "w" )
                    backup.writelines( lines )
                    backup.close()

                    fo.close()
                    fo = open( dirname + "/" + file, "w" )
                    fo.writelines( newlines )

                    print "File " + dirname + "/" + file + " was changed."
                fo.close()

def fnHelp():
    helpstr = """
This program deletes all the 'dangling' spaces at the end
of all the lines in all the files with the passed extensions
within the directory tree from whose root it was called
(i.e. recurses into sub-directories)

Syntax:
    rem_endspc [ext1] [ext2] [...]

Extensions are given in the form:
    .cpp .h .txt (no asterisk at the beginning)

Copyright (c) 2002 by Adrian Cearnau (cearny@cearny.ro)
Any use, commercial or not, is allowed
"""
    print helpstr

if len(sys.argv) < 2:
    fnHelp()
else:
    exts = sys.argv[1].split()
    currdir = os.getcwd()
    os.path.walk( currdir, fnParse, exts )
    #print "\nTotally", num_files, "files were patched\n"
    print "done"