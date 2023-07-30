//**************************************************************************
// file name: SignalAnalyzer.h
//**************************************************************************
//_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
// This class implements a signal processing block known as a signal
// analyzer.  Given 8-bit IQ samples from an SDR, plots can be displayed
// of the magnitude of the signal or the power spectrum of the signal.
///_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

#ifndef __SIGNALANALYZER__
#define __SIGNALANALYZER__

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#include <fftw3.h>

#include <X11/Xlib.h>

enum DisplayType {SignalMagnitude=1, PowerSpectrum};

class SignalAnalyzer
{
  //***************************** operations **************************

  public:

  SignalAnalyzer(DisplayType displayType,int windowWidthInPixels,
                 int windowHeightInPixels);

 ~SignalAnalyzer(void);

  void drawGridlines(void);
  void plotSignalMagnitude(int8_t *signalBufferPtr,uint32_t bufferLength);
  void plotPowerSpectrum(int8_t *signalBufferPtr,uint32_t bufferLength);

  private:

  //*******************************************************************
  // Utility functions.
  //*******************************************************************
  void initializeFftw(void);
  void initializeX(void);

  uint32_t computeSignalMagnitude(int8_t *signalBufferPtr,
                                  uint32_t bufferLength);

  uint32_t computePowerSpectrum(int8_t *signalBufferPtr,
                                uint32_t bufferLength);

  //*******************************************************************
  // Attributes.
  //*******************************************************************
  DisplayType displayType;

  // We ulitmately map values to these pixels.
  int windowWidthInPixels;
  int windowHeightInPixels;

  // This is used for plotting of signals.
  XPoint points[16384];

  // This is used for signal magnitude results.
  int16_t magnitudeBuffer[16384];

  // This will be used to swap the upper and lower halves of an array.
  uint32_t fftShiftTable[8192];

  // FFTW3 support.
  fftw_complex *fftInputPtr;
  fftw_complex *fftOutputPtr;
  fftw_plan fftPlan;

  // Xlib support.
  Display *displayPtr;
  Window window;
  GC graphicsContext;
};

#endif // __SIGNALANALYZER__
