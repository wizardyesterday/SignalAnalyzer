//************************************************************************
// file name: SignalAnalyzer.cc
//************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

#include "SignalAnalyzer.h"

using namespace std;

#define N (8192)

/*****************************************************************************

  Name: XErrorCallback

  Purpose: The purpose of this function is to to handle errors that
  are generated by X.

  Calling Sequence: XErrorCallback(displayPtr,errorPtr)
 
  Inputs:

    displayPtr - A pointer to the display for which the error was
    generated.

    errorPtr - A pointer to the error event for which the error
    was generated.

 Outputs:

    None.

*****************************************************************************/
static int XErrorCallback(Display *displayPtr,XErrorEvent *errorPtr)
{
  char msg[80];

  // Retrieve the error text
  XGetErrorText (displayPtr,errorPtr->error_code,msg,80);

  // Display the error message to the user.
  fprintf(stderr,"%s\n",msg);

  return (0);

} // XErrorCallback

/*****************************************************************************

  Name: SignalAnalyzer

  Purpose: The purpose of this function is to serve as the constructor for
  an instance of an SignalAnalyzer.

  Calling Sequence: SignalAnalyzer(windowWidthInPixels,
                                   windowHeightInPixels)
 
  Inputs:

    windowWidthInPixels - The number of pixels in the horizontal
    direction.

    windowHeightInPixels - The number of pixels in the vertical
    direction.

 Outputs:

    None.

*****************************************************************************/
SignalAnalyzer::SignalAnalyzer(int windowWidthInPixels,
                               int windowHeightInPixels)
{
  uint32_t i;

  // Retrieve for later use.
  this->windowWidthInPixels = windowWidthInPixels;
  this->windowHeightInPixels = windowHeightInPixels;

  // Let's force this.
  windowWidthInPixels = 1024;
  windowHeightInPixels = 256;

  // Set up the FFT stuff.
  initializeFftw();

  // Do all the cool stuff for X.
  initializeX();

  return;

} // SignalAnalyzer

/*****************************************************************************

  Name: ~SignalAnalyzer

  Purpose: The purpose of this function is to serve as the destructor for
  an instance of an SignalAnalyzer.

  Calling Sequence: ~SignalAnalyzer()

  Inputs:

    None.

  Outputs:

    None.

*****************************************************************************/
SignalAnalyzer::~SignalAnalyzer(void)
{

  // We're done with this display.
  XCloseDisplay(displayPtr);

  // Release FFT resources.
  fftw_destroy_plan(fftPlan);
  fftw_free(fftInputPtr);
  fftw_free(fftOutputPtr);

  return;

} // ~SignalAnalyzer

/*****************************************************************************

  Name: initializeFftw

  Purpose: The purpose of this function is to initialize FFTW so that
  FFT's can be performed.

  Calling Sequence: initializeFftw()

  Inputs:

    None.

 Outputs:

    None.

*****************************************************************************/
void SignalAnalyzer::initializeFftw(void)
{
  uint32_t i;

  // Construct the permuted indices.
  for (i = 0; i < N/2; i++)
  {
    fftShiftTable[i] = i + N/2;
    fftShiftTable[i + N/2] = i;
  } // for

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // This block of code sets up FFTW for a size of 8192 points.  This
  // is the result of N being defined as 8192.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  fftInputPtr = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*N);
  fftOutputPtr = (fftw_complex *)fftw_malloc(sizeof(fftw_complex)*N);

  fftPlan = fftw_plan_dft_1d(N,fftInputPtr,fftOutputPtr,
                             FFTW_FORWARD,FFTW_ESTIMATE);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return;

} // initializeFftw

/*****************************************************************************

  Name: initializeX

  Purpose: The purpose of this function is to perform all of the
  initialization that is related to launching an X application.

  Calling Sequence: initializeX()

  Inputs:

    None.

 Outputs:

    None.

*****************************************************************************/
void SignalAnalyzer::initializeX(void)
{
  int blackColor;
  int whiteColor;
  XEvent event;

 //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Setup X.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Register the X error handler.
  XSetErrorHandler(XErrorCallback);

  // Connect to the X server.
  displayPtr = XOpenDisplay(NULL);

  // Retrieve these two colors.
  blackColor = BlackPixel(displayPtr,DefaultScreen(displayPtr));
  whiteColor = WhitePixel(displayPtr,DefaultScreen(displayPtr));

  // Create the window.
  window = XCreateSimpleWindow(displayPtr,
                               DefaultRootWindow(displayPtr),
                               0,
                               0, 
                               windowWidthInPixels,
                               windowHeightInPixels,
                               0,
                               blackColor,
                               blackColor);

  // We want to get MapNotify events.
  XSelectInput(displayPtr,window,StructureNotifyMask);

  // Create a "Graphics Context".
  graphicsContext = XCreateGC(displayPtr,window,0,NULL);

  // Tell the GC we draw using the white color.
  XSetForeground(displayPtr,graphicsContext,whiteColor);

  // "Map" the window (that is, make it appear on the screen).
  XMapWindow(displayPtr,window);

  // Wait for the MapNotify event.
  for(;;)
  {
    XNextEvent(displayPtr, &event);

    if (event.type == MapNotify)
    {
      break;
    } // if);
  } // for
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return;

} // initializeX

/*****************************************************************************

  Name: drawGridlines

  Purpose: The purpose of this function is to draw a grid over the
  analyzer display.

  Calling Sequence: drawGridlines()

  Inputs:

    None.

 Outputs:

    None.

*****************************************************************************/
void SignalAnalyzer::drawGridlines(void)
{
  uint32_t i;

  // Draw vertical lines.
  for (i = 1; i < 16; i++)
  {
    XDrawLine(displayPtr,
    window,
    graphicsContext,i*64,0,i*64,windowHeightInPixels);
  } // for

  // Draw horozontal lines.
  for (i = 1; i < 4; i++)
  {
    XDrawLine(displayPtr,
    window,
    graphicsContext,i,i*64,windowWidthInPixels,i*64);
  } // for

  return;

} // drawGridLines

/*****************************************************************************

  Name: plotSignalMagnitude

  Purpose: The purpose of this function is to perform a magnitude plot
  of IQ data to the display.

  Calling Sequence: plotSignalMagnitude(signalBufferPtr,bufferLength)

  Inputs:

    signalBufferPtr - A pointer to a buffer of IQ data.  The buffer is
    formatted with interleaved data as: I1,Q1,I2,Q2,...

    bufferLength - The number of values in the signal buffer.  This
    represents the total number of items in the buffer, rather than
    the number of IQ sample pairs in the buffer.

 Outputs:

    None.

*****************************************************************************/
void SignalAnalyzer::plotSignalMagnitude(
  int8_t *signalBufferPtr,
  uint32_t bufferLength)
{
  uint32_t i;
  uint32_t j;

  bufferLength = computeSignalMagnitude(signalBufferPtr,bufferLength);

  // Reference the starts of the points array.
  j = 0;

  // We're fitting an 8192-point FFT to a 1024 pixel display width.
  for (i = 0; i < bufferLength; i += 8)
  {
    points[j].x = (short)j;
    points[j].y = windowHeightInPixels - magnitudeBuffer[i];

    // Reference the next storage location.
    j++;
  } // for

  // Erase the previous plot.
  XClearWindow(displayPtr,window);

  // Make this display pretty.
  drawGridlines();

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Annotate the display.  This is really too
  // sensitive to fonts.  I'll think of something
  // later.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  XDrawString(displayPtr,window,graphicsContext,
              768,20,
              "Sweep Time: 32ms",16);

  XDrawString(displayPtr,window,graphicsContext,
              768,35,
              "2 ms/div",8);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Plot the signal.
  XDrawLines(displayPtr,
             window,
             graphicsContext,
             points,windowWidthInPixels,
             CoordModeOrigin);

  // Send the request to the server
  XFlush(displayPtr);

  return;

} // plotSignalMagnitude

/*****************************************************************************

  Name: plotPowerSpectrum

  Purpose: The purpose of this function is to perform a power spectrum plot
  of IQ data to the display.

  Calling Sequence: plotPowerSpectrum(signalBufferPtr,bufferLength)

  Inputs:

    signalBufferPtr - A pointer to a buffer of IQ data.  The buffer is
    formatted with interleaved data as: I1,Q1,I2,Q2,...

    bufferLength - The number of values in the signal buffer.  This
    represents the total number of items in the buffer, rather than
    the number of IQ sample pairs in the buffer.

 Outputs:

    None.

*****************************************************************************/
void SignalAnalyzer::plotPowerSpectrum(
  int8_t *signalBufferPtr,
  uint32_t bufferLength)
{
  uint32_t i;
  uint32_t j;

  bufferLength = computePowerSpectrum(signalBufferPtr,bufferLength);

  // Reference the starts of the points array.
  j = 0;

  // We're fitting an 8192-point FFT to a 1024 pixel display width.
  for (i = 0; i < bufferLength; i += 8)
  {
    points[j].x = (short)j;
    points[j].y = windowHeightInPixels - magnitudeBuffer[i];

    // Reference the next storage location.
    j++;
  } // for

  // Erase the previous plot.
  XClearWindow(displayPtr,window);

  // Make this display pretty.
  drawGridlines();

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Annotate the display.  This is really too
  // sensitive to fonts.  I'll think of something
  // later.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  XDrawString(displayPtr,window,graphicsContext,
              768,20,
              "Frequency Span: 256kHz",22);

  XDrawString(displayPtr,window,graphicsContext,
              768,35,
              "16 kHz/div",10);
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Plot the signal.
  XDrawLines(displayPtr,
             window,
             graphicsContext,
             points,windowWidthInPixels,
             CoordModeOrigin);

  // Send the request to the server
  XFlush(displayPtr);

  return;

} // plotPowerSpectrum

/*****************************************************************************

  Name: computeSignalMagnitude

  Purpose: The purpose of this function is to compute the magnitude of
  IQ data.

  Calling Sequence: numberOfSamples = computeSignalMagnitude(
                                         signalBufferPtr,
                                         bufferLength)

  Inputs:

    signalBufferPtr - A pointer to a buffer of IQ data.  The buffer is
    formatted with interleaved data as: I1,Q1,I2,Q2,...

    bufferLength - The number of values in the signal buffer.  This
    represents the total number of items in the buffer, rather than
    the number of IQ sample pairs in the buffer.

 Outputs:

    None.

*****************************************************************************/
uint32_t SignalAnalyzer::computeSignalMagnitude(
  int8_t *signalBufferPtr,
  uint32_t bufferLength)
{
  uint32_t i;
  uint32_t magnitudeIndex;
  int16_t iMagnitude, qMagnitude;

  // Reference the beginning of the magnitude buffer.
  magnitudeIndex = 0;

  for (i = 0; i < bufferLength; i += 2)
  {
    // Grab these values for the magnitude estimator.
    iMagnitude = abs((int16_t)signalBufferPtr[i]);
    qMagnitude = abs((int16_t)signalBufferPtr[i+1]);

    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    // This block of code performs a magnitude estimation of
    // the complex signal.
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
    if (iMagnitude > qMagnitude)
    {
      magnitudeBuffer[magnitudeIndex] = iMagnitude  + (qMagnitude >> 1);
    } // if
    else
    {
      magnitudeBuffer[magnitudeIndex] = qMagnitude + (iMagnitude >> 1);
    } // else
    //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

    // Reference the next storage location.
    magnitudeIndex++;
  } // for

  // Swap the array halfs.

  return (bufferLength / 2);

} // computeSignalMagnitude

/*****************************************************************************

  Name: computePowerSpectrum

  Purpose: The purpose of this function is to compute the power spectrum
  of IQ data.

  Calling Sequence: computePowerSpectrum(signalBufferPtr,bufferLength)

  Inputs:

    signalBufferPtr - A pointer to a buffer of IQ data.  The buffer is
    formatted with interleaved data as: I1,Q1,I2,Q2,...

    bufferLength - The number of values in the signal buffer.  This
    represents the total number of items in the buffer, rather than
    the number of IQ sample pairs in the buffer.

 Outputs:

    None.

*****************************************************************************/
uint32_t SignalAnalyzer::computePowerSpectrum(
  int8_t *signalBufferPtr,
  uint32_t bufferLength)
{
  uint32_t i;
  uint32_t j;
  int16_t temp;
  int16_t iMagnitude, qMagnitude;
  double power;
  double powerInDb;
  double iK, qK;

  // Reference the beginning of the FFT buffer.
  j = 0;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Fill up the input array.  The second index of the
  // array is used as follows: a value of 0 references
  // the real component of the signal, and a value of 1
  // references the imaginary component of the signal. 
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  for (i = 0; i < bufferLength; i += 2)
  {
    // Store the real value.
    fftInputPtr[j][0] = (double)signalBufferPtr[i];

    // Store the comple value.
    fftInputPtr[j][1] = (double)signalBufferPtr[i+1];

    // Reference the next storage location.
    j += 1; 
  } // for
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Compute the DFT.
  fftw_execute(fftPlan);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Compute the magnitude of the spectrum in decibels.
  // Originally, I used a simple approximation for the
  // magnitude, but the result was a lousy display of the
  // spectrum.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  for (i = 0; i < N; i++)
  {
    // Retrive the in-phase and quadrature parts.
    iK = fftOutputPtr[i][0];
    qK = fftOutputPtr[i][1];

    // Compute signal power, |I + jQ|.
    power = (iK * iK) + (qK * qK);

    // We want power in decibels.
    powerInDb = 10*log10(power);

    //--------------------------------------------
    // The fftShiftTable[] allows us to store
    // the FFT output values such that the
    // center frequency bin is in the center
    // of the output array.  This results in a
    // display that looks like that of a spectrum
    // analyzer.
    //--------------------------------------------
    j = fftShiftTable[i];
    //--------------------------------------------

    // We're reusabing the magnitude buffer for power values.
    magnitudeBuffer[j] = (int16_t)powerInDb; 
  } // for
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (bufferLength / 2);

} // computePowerSpectrum


