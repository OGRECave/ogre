#!/usr/bin/python3

import sys
import os.path
from lxml import etree

mesh_dtd = etree.DTD(sys.path[0]+"/ogremeshxml.dtd")
skel_dtd = etree.DTD(sys.path[0]+"/ogreskeletonxml.dtd")

mesh = sys.argv[1]
meshxml = etree.parse(sys.argv[1])

ret = 0

if mesh_dtd.validate(meshxml):
    print("mesh validation successful")
else:
    print("mesh validation errors")
    print(mesh_dtd.error_log.filter_from_errors())
    ret = -1

skel = meshxml.find("skeletonlink")

if skel is None:
    sys.exit(ret)

skelxml = etree.parse("{}/{}.xml".format(os.path.dirname(sys.argv[1]), skel.attrib["name"]))
if skel_dtd.validate(skelxml):
    print("skeleton validation successful")
else:
    print("skeleton validation errors")
    print(skel_dtd.error_log.filter_from_errors())
    ret = -1

sys.exit(ret)

