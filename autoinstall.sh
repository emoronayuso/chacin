#!/bin/sh

aclocal && autoconf && automake --add-missing && autoconf && ./configure && make && sudo make && make clean
