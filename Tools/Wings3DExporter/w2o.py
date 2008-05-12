#!/usr/bin/python

from erlang_ext import *
import types
import pprint

import io3d_wings
import io3d_ogre

import getopt

version = "0.93"

def conv(infile, outfile, writeImages, keepRotation, scaleFactor):
	obj = io3d_wings.read_wings(infile, writeImages, keepRotation)
	if scaleFactor != 1.0: obj.scale(scaleFactor)
	io3d_ogre.write_ogre(obj, outfile)

if __name__ == "__main__":

	try:
		options, args = getopt.getopt(sys.argv[1:], "hviks:")

		writeImages = 0
		keepRotation = 0
		scaleFactor = 1.0

		for o in options:
			option, value = o

			if option == '-h':
				print """Usage:
w2o [-iks] file1.wings file2.wings

Options:
    -h     This help
    -v     Print version
    -i     Export images from the wings file via PIL
    -k     Keep the coordinates as they are, do not correct them
           to the OGRE coordinate system. Use this if you
           want to rotate your objects around the X axis in
           code or if you already rotated the objects in Wings.
    -s n   Scale the object uniformly using the given floating 
           point factor.
"""
				sys.exit(1)

			elif option == '-v':
				print "w2o", version
				sys.exit(1)

			elif option == '-i': writeImages = 1

			elif option == '-k': keepRotation = 1

			elif option == '-s': scaleFactor = float(value)

		for arg in args:

			# process filename
			if arg[-6:] == ".wings":
				dstname = arg[:-6]
			else:
				dstname = arg
			dstname += ".mesh.xml"
			conv(arg, dstname, writeImages, keepRotation, scaleFactor)

	except getopt.GetoptError, e:
		print e
		print "try -h for help"
		sys.exit(1)

