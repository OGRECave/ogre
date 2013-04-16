$?EXTRACFLAGS=
FLASCC:=/Users/wolfmanfx/flashcc/sdk
FLEX:=/Users/wolfmanfx/flashcc/flex
CMAKE_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
BUILD_DIR := $(CURDIR)/buildFlash

$?UNAME=$(shell uname -s)
ifneq (,$(findstring CYGWIN,$(UNAME)))
	$?nativepath=$(shell cygpath -at mixed $(1))
	$?unixpath=$(shell cygpath -at unix $(1))
else
	$?nativepath=$(abspath $(1))
	$?unixpath=$(abspath $(1))
endif
$?AS3COMPILERARGS=java $(JVMARGS) -jar $(call nativepath,$(FLASCC)/usr/lib/$(AS3COMPILER)) -merge -md

all: check
	mkdir -p "$(BUILD_DIR)"
	cd $(BUILD_DIR) && PATH="$(call unixpath,$(FLASCC)/usr/bin):$(PATH)" CC=gcc CXX=g++ CFLAGS="-O4 $(EXTRACFLAGS)" CXXFLAGS="-O4 $(EXTRACFLAGS)" cmake -DFLASHCC=1 ..
	cd buildflash && PATH="$(call unixpath,$(FLASCC)/usr/bin):$(PATH)" make -j8

check:
	@if [ -d $(FLASCC)/usr/bin ] ; then true ; \
	else echo "Couldn't locate FLASCC sdk directory, please invoke make with \"make FLASCC=/path/to/FLASCC/sdk ...\"" ; exit 1 ; \
	fi

	@if [ -d "$(FLEX)/bin" ] ; then true ; \
	else echo "Couldn't locate Flex sdk directory, please invoke make with \"make FLEX=/path/to/flex  ...\"" ; exit 1 ; \
	fi

clean:
	rm -rf $(BUILD_DIR) *.swf *.swc
