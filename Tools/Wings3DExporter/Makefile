
.PHONY: dist

DIST_NAME = wings-ogre
VERSION = 0.93
FULL_NAME = $(DIST_NAME)-$(VERSION)

dist:
	-mkdir $(FULL_NAME)
	cp README.txt $(FULL_NAME)/
	cp xmlout.py $(FULL_NAME)/
	cp Makefile $(FULL_NAME)/
	cp erlang_ext.py $(FULL_NAME)/
	cp vector.py mesh.py pgon.py $(FULL_NAME)/
	cp io3d_wings.py io3d_ogre.py $(FULL_NAME)/
	cp w2o.py $(FULL_NAME)/
	tar -czf $(FULL_NAME).tar.gz $(FULL_NAME)
	-rm -rf $(FULL_NAME)

