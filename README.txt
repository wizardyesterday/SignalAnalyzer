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


