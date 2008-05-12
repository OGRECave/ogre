import sys
import os
import getopt
import string

def fnParse( arg, dirname, files ):
    for file in files:
        for ext in arg:
            if file.rfind( ext, len(file)-len(ext), len(file) ) != -1 and not os.path.isdir( dirname + "/" + file ):
                fo = open( dirname + "/" + file, "rb" )
                lines = fo.read()
                if not '\0' in lines and lines[len(lines)-1] != '\n':
                    newlines = lines + '\n'
                    fo.close()
                    fo = open( dirname + "/" + file, "wb" )
                    fo.write(newlines)
                    print "File " + dirname + "/" + file + " was changed."
                fo.close()

def fnHelp():
    helpstr = """
This nifty little program adds a CR/LF at the end of every
file with the given extension within the directory tree
from where it was called (i.e. recurses in sub-directories).

Syntax:
    add_crlf [ext1] [ext2] [...]

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