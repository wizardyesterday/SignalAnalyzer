#!/bin/sh

g++ -O0 -g -Iinclude -o analyzer src/analyzer.cc src/SignalAnalyzer.cc -L/usr/X11R6/lib -lX11

