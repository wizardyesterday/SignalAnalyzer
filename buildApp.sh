#!/bin/sh

g++ -O3 -Iinclude -o analyzer src/analyzer.cc src/SignalAnalyzer.cc -L/usr/X11R6/lib -lX11 -l fftw3

