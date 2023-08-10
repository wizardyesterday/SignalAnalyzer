#!/bin/sh

g++ -O2 -Iinclude -o analyzer src/analyzer.cc src/SignalAnalyzer.cc -L/usr/X11R6/lib -lX11 -l fftw3

g++ -O2 -Iinclude -o fileThrottler src/fileThrottler.cc

