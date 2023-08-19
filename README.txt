After performing analysis of IQ data captures, from my rtlsdr, I thought it
was cool to load an IQ file, segment at a time, and review the power
spectrum of the data.  This motivated me to create my signal anzlyzer
app so that I could review this information (power spectrum or time
domain view of IQ data).

This app accepts IQ data from stdin and plot the data.  I'm new to writing
C++ (or C) programs that plot data.  Due to my ancient hardware and desire
for bloat-free software, I decided to use xlib.

At this moment, the output is somewhat crude, but it does the job for me.
The initial version of this software does time domain display of IQ data.  I
should have power spectral plots in a few days (I hope).  I'll be using the
fftw3 library to perform the fast Fourier transforms, so I have a slight
learning curve.

Chris G. 07/26/2023

------------------------------------------------------------------------
08/01/2023
------------------------------------------------------------------------
The system has been completed, and the diaplay is cleaned up.

Lessons learned:

1. The log10() function is very expensive in terms of CPU cycles (no
surprise).

2. The Scilab fftshift() output maade it easy to set up a table that
allowed me to store FFT output magnitude in the magnitude array such
that the upper and lower halves of the array are swapped.

3. When you're running X in 16-bit display mode, make sure you
map rgb colors to MSB: red[4:0] green[5:0], blue[4:0]: LSB.  Once
I write an rgbYo16Bit() function (and used it), display colors
rendered properly.  Before I did this, colors were all wrong.

4. XDrawLine() (for drawing grids) and XDrawLines() (for plotting signals)
are your friends.  So is XDrawString() for placing text on the display.
I'm not sure how different fonts on different systems will mess things up.


------------------------------------------------------------------------
08/10/2023
------------------------------------------------------------------------
We now have a file throttler that provides the ability to read from
stdin, delay for a specified day, and output what was just read
from stdin to stdout.
This provides the ability to play back IQ data files in real time...
well sort of.  On a multitasking system such as Linux, there will
be extra latency in the delay.  Even with this restriction, you can
output data to my signal analyzer, and view the spectrum in "sort of"
real time.

------------------------------------------------------------------------
08/18/2023
------------------------------------------------------------------------
We now have a Lissajous scope.  This provides the capability to see
the quality of the IQ data.

