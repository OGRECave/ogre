#!/usr/bin/python3

import sys
from lxml import etree

dtd = etree.DTD(open("dotscene.dtd"))
root = etree.parse(sys.argv[1])

if dtd.validate(root):
    print("validation successful")
else:
    print(dtd.error_log.filter_from_errors())