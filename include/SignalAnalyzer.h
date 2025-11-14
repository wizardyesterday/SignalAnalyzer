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

// This is the FFT size.
#define N (8192)

enum DisplayType {SignalMagnitude=1, PowerSpectrum, Lissajous};

class SignalAnalyzer
{
  //***************************** operations **************************

  public:

  SignalAnalyzer(DisplayType displayType,
      float sampleRate,
      float verticalGain,
      int32_t baselineInDb);

 ~SignalAnalyzer(void);

  void plotSignalMagnitude(int8_t *signalBufferPtr,uint32_t bufferLength);
  void plotPowerSpectrum(int8_t *signalBufferPtr,uint32_t bufferLength);
  void plotLissajous(int8_t *signalBufferPtr,uint32_t bufferLength);

  private:

  //*******************************************************************
  // Utility functions.
  //*******************************************************************
  void initializeFftw(void);
  void initializeX(void);
  void initializeAnnotationParameters(float sampleRate);
  void drawGridlines(void);

  uint32_t computeSignalMagnitude(int8_t *signalBufferPtr,
                                  uint32_t bufferLength);

  uint32_t computeLogPowerSpectrum(int8_t *signalBufferPtr,
                                   uint32_t bufferLength);

  //*******************************************************************
  // Attributes.
  //*******************************************************************
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Display support.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  DisplayType displayType;
  unsigned long scopeBackgroundColor;
  unsigned long scopeGridColor;
  unsigned long scopeSignalColor;

  char sweepTimeBuffer[80];
  char sweepTimeDivBuffer[80];
  char frequencySpanBuffer[80];
  char frequencySpanDivBuffer[80];
  char sampleRateBuffer[80];
  char lissajousDivBuffer[80];

  int annotationHorizontalPosition;
  int annotationFirstLinePosition;
  int annotationSecondLinePosition;

  uint32_t spectrumStride;
  uint32_t signalStride;
  float verticalGain;
  int32_t baselineInDb;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // We ulitmately map values to these pixels.
  int windowWidthInPixels;
  int windowHeightInPixels;

  // This is used for plotting of signals.
  XPoint points[1024];

  // This is used for signal magnitude results.
  int16_t magnitudeBuffer[N];

  // This will be used to swap the upper and lower halves of an array.
  uint32_t fftShiftTable[N];

  // This will be used for windowing data before the FFT.
  double hanningWindow[N];

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
