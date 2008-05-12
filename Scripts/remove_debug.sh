#!/bin/sh
sed "s/ -g//" $1 > $1.new
mv -f $1.new $1 
