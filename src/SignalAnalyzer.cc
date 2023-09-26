//************************************************************************
// file name: SignalAnalyzer.cc
//************************************************************************
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>

#include "SignalAnalyzer.h"

using namespace std;

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

  Calling Sequence: SignalAnalyzer(sampleRate)
 
  Inputs:

    sampleRate - The sample rate of incoming IQ data in units of S/s.

 Outputs:

    None.

*****************************************************************************/
SignalAnalyzer::SignalAnalyzer(DisplayType displayType,float sampleRate)
{
  uint32_t i;

  if (sampleRate <= 0)
  {
    // Keep it sane.
    sampleRate = 256000;
  } // if

  // Retrieve for later use.
  this->displayType = displayType;

  // This is the display dimensions in pixels.
  windowWidthInPixels = 1024;
  windowHeightInPixels = 256;

  // Set strides.
  spectrumStride = N / windowWidthInPixels;
  signalStride = N / windowWidthInPixels;

  // Construct the Hanning window array.
  for (i = 0; i < N; i++)
  {
    hanningWindow[i] = 0.5 - 0.5 * cos((2 * M_PI * i)/N);
  } // for

  // Set up the FFT stuff.
  initializeFftw();

  // Do all the cool stuff for X.
  initializeX();

  // This sets up information stuff on the scopes.
  initializeAnnotationParameters(sampleRate);

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

  Note: For a 16-bit display, the partitioning of the color values
  are as follows (with r = red, g = green, blue = blue):

  r[4:0], g[5:0], blue[4:0] = rrrrr gggggg bbbbb = 16 bits

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
  int screen;
  Colormap colormap;
  XColor exact;
  XColor closest;
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

  //-------------------------------------------------------
  // Initialize colors.  These colors will be used
  // throughout the application.
  //-------------------------------------------------------
  // We need these for the color lookup invocations.
  screen = DefaultScreen(displayPtr);
  colormap = DefaultColormap(displayPtr,screen);

  // Background is midnight blue.
  XAllocNamedColor(displayPtr,colormap,"midnight blue",&exact,&closest);
  scopeBackgroundColor = closest.pixel;

  // Grid is yellow.
  XAllocNamedColor(displayPtr,colormap,"yellow",&exact,&closest);
  scopeGridColor = closest.pixel;

  // Signal is green.
  XAllocNamedColor(displayPtr,colormap,"green",&exact,&closest);
  scopeSignalColor = closest.pixel;
  //-------------------------------------------------------

  // Create the window.
  window = XCreateSimpleWindow(displayPtr,
                               DefaultRootWindow(displayPtr),
                               0,
                               0, 
                               windowWidthInPixels,
                               windowHeightInPixels,
                               0,
                               blackColor,
                               scopeBackgroundColor);

  // We want to get MapNotify events.
  XSelectInput(displayPtr,window,StructureNotifyMask);

  // Create a "Graphics Context".
  graphicsContext = XCreateGC(displayPtr,window,0,NULL);

  // First-time foreground is white.
  XSetForeground(displayPtr,graphicsContext,whiteColor);

  switch (displayType)			
  {
    case SignalMagnitude:
    {
      XStoreName(displayPtr,window,"Oscilloscope");
      break;
    } // case

    case PowerSpectrum:
    {
      XStoreName(displayPtr,window,"Spectrum Analyzer");
      break;
    } // case

    case Lissajous:
    {
      XStoreName(displayPtr,window,"Lissajous Scope");
      break;
    } // case

    default:
    {
      XStoreName(displayPtr,window,"Signal Analyzer");
      break;
    } // case
  } // switch

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

  // Send the request to the server
  XFlush(displayPtr);
 
  return;

} // initializeX

/*****************************************************************************

  Name: initializeAnnotationParameters

  Purpose: The purpose of this function is to set up the scope annotations.
  Examples are sweep time and frequency span.

  Calling Sequence: initializeAnnotationParameters(sampleRate)

  Inputs:

    sampleRate - The sample rate of incoming IQ data in units of S/s.

 Outputs:

    None.

*****************************************************************************/
void SignalAnalyzer::initializeAnnotationParameters(float sampleRate)
{
  float sweepTimeInMs;
  float frequencySpanInKHz;
  float sampleRateInKHz;
  int fontHeight;
  int fontWidth;
  XFontStruct *fontInfoPtr;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // This is our font stuff.  By experiment, I see that
  // fixed fonts are 9 pixels high and 6 pixels wide.  Now,
  // the starting horizontal postion of the annotations is
  // 180 pixels to the left or the right boundary of the
  // window.  That means that the maximum amount of
  // characters is 180 / 6 = 30 - some spare change = 29
  // characters.  Not too shoddy.  If you need more, just
  // adjust the number.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // This is deterministic with respect to size.
  fontInfoPtr = XLoadQueryFont(displayPtr,"fixed");

  XSetFont(displayPtr,
           graphicsContext,
           fontInfoPtr->fid);

  // Compute the height and width of the font.
  fontHeight = fontInfoPtr->ascent - fontInfoPtr->descent;
  fontWidth = XTextWidth(fontInfoPtr,"A",strlen("A"));

  // Set our vertical positions of the annotations.
  annotationFirstLinePosition = fontHeight + 6;
  annotationSecondLinePosition = annotationFirstLinePosition + 15;

  //-------------------------------------------------------
  // This is where the annotation will start.  Remember
  // that you can have (180 / 6) - 1 = 29 characters.
  // This is where you adjust things.  Also remember
  // that, for a fixed font, the pixel height is 9 pixels,
  // and the pixel width is 6 pixels.  This will save you
  // lots of grief when adjusting things.
  //-------------------------------------------------------
  annotationHorizontalPosition = windowWidthInPixels - 180;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Set up annotations.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Compute sweep time in milliseconds.
  sweepTimeInMs = N / sampleRate;
  sweepTimeInMs *= 1000;

  // Compute frequency span in kHz.
  frequencySpanInKHz = sampleRate / 1000;
  sampleRateInKHz = sampleRate / 1000;

  // Save in buffers to be displayed in oscilloscope.
  sprintf(sweepTimeBuffer,"Sweep Time: %.2fms",sweepTimeInMs);
  sprintf(sweepTimeDivBuffer,"%.2fms/div",sweepTimeInMs/16);

  // Save in buffers to be displayed in spectrum analyzer.
  sprintf(frequencySpanBuffer,"Frequency Span: %.2fkHz",frequencySpanInKHz);
  sprintf(frequencySpanDivBuffer,"%.2fkHz/div",frequencySpanInKHz/16);

  // Save in buffers to displayed in Lissajous scope.
  sprintf(sampleRateBuffer,"Sample Rate: %.2fkHz",sampleRateInKHz);
  sprintf(lissajousDivBuffer,"64units/div");
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return;

} // initialize annotationParameters

/*****************************************************************************

  Name: drawGridlines

  Purpose: The purpose of this function is to draw a grid on the
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
  int deltaH;
  int deltaV;
  int horizontalPosition;
  int verticalPosition;

  // Compute grid spacing.
  deltaH = windowWidthInPixels / 16;
  deltaV = windowHeightInPixels / 4;

  // Set the grid color.
  XSetForeground(displayPtr,graphicsContext,scopeGridColor);

  // Compute horizontal position of first vertical grid.
  horizontalPosition = deltaH;

  // Draw vertical lines.
  for (i = 1; i < 16; i++)
  {
    XDrawLine(displayPtr,
              window,
              graphicsContext,
              horizontalPosition,
              0,
              horizontalPosition,
              windowHeightInPixels);

    // Compute next horizontal position of vertical grid.
    horizontalPosition += deltaH;
  } // for

  // Compute vertical position of first horizontal grid.
  verticalPosition = deltaV;

  // Draw horozontal lines.
  for (i = 1; i < 4; i++)
  {
    XDrawLine(displayPtr,
              window,
              graphicsContext,
              0,
              verticalPosition,
              windowWidthInPixels,
              verticalPosition);

    // Compute next vertical position of horizontal grid.
    verticalPosition += deltaV;
  } // for

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Nicely mark the horizontal center.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  //---------------------------------
  // Place marks on upper part of
  // screen.
  //---------------------------------
  XDrawLine(displayPtr,
            window,
            graphicsContext,
            (windowWidthInPixels/2) - 1,
            0,
            (windowWidthInPixels/2) - 1,
            5);

  XDrawLine(displayPtr,
            window,
            graphicsContext,
            (windowWidthInPixels/2) + 1,
            0,
            (windowWidthInPixels/2) + 1,
            5);

  //---------------------------------
  // Place marks on lower part of
  // screen.
  //---------------------------------
  XDrawLine(displayPtr,
            window,
            graphicsContext,
            (windowWidthInPixels/2) - 1,
            windowHeightInPixels-5,
            (windowWidthInPixels/2) - 1,
            windowHeightInPixels);

  XDrawLine(displayPtr,
            window,
            graphicsContext,
            (windowWidthInPixels/2) + 1,
            windowHeightInPixels-5,
            (windowWidthInPixels/2) + 1,
            windowHeightInPixels);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Send the request to the server
  XFlush(displayPtr);
 
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

  // Reference the start of the points array.
  j = 0;

  // We're fitting an 8192 IQ samples to the display width.
  for (i = 0; i < bufferLength; i += signalStride)
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

  // Set the signal color.
  XSetForeground(displayPtr,graphicsContext,scopeSignalColor);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Annotate the display.  This is really too
  // sensitive to fonts.  I'll think of something
  // later.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  XDrawString(displayPtr,window,graphicsContext,
              annotationHorizontalPosition,
              annotationFirstLinePosition,
              sweepTimeBuffer,strlen(sweepTimeBuffer));

  XDrawString(displayPtr,window,graphicsContext,
              annotationHorizontalPosition,
              annotationSecondLinePosition,
              sweepTimeDivBuffer,strlen(sweepTimeDivBuffer));
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

  // Reference the start of the points array.
  j = 0;

  // We're fitting an 8192-point FFT to the display width.
  for (i = 0; i < bufferLength; i += spectrumStride)
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

  // Set the signal color.
  XSetForeground(displayPtr,graphicsContext,scopeSignalColor);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Annotate the display.  This is really too
  // sensitive to fonts.  I'll think of something
  // later.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  XDrawString(displayPtr,window,graphicsContext,
              annotationHorizontalPosition,
              annotationFirstLinePosition,
              frequencySpanBuffer,strlen(frequencySpanBuffer));

  XDrawString(displayPtr,window,graphicsContext,
              annotationHorizontalPosition,
              annotationSecondLinePosition,
              frequencySpanDivBuffer,strlen(frequencySpanDivBuffer));
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

  Name: plotLissajous

  Purpose: The purpose of this function is to perform a Lissajous plot
  of IQ data to the display.  This provides a nice indication as to
  whether or not the IQ data samples are clipping.  Clipping is indicated
  by a square pattern.

  Calling Sequence: plotLissajous(signalBufferPtr,bufferLength)

  Inputs:

    signalBufferPtr - A pointer to a buffer of IQ data.  The buffer is
    formatted with interleaved data as: I1,Q1,I2,Q2,...

    bufferLength - The number of values in the signal buffer.  This
    represents the total number of items in the buffer, rather than
    the number of IQ sample pairs in the buffer.

 Outputs:

    None.

*****************************************************************************/
void SignalAnalyzer::plotLissajous(
  int8_t *signalBufferPtr,
  uint32_t bufferLength)
{
  uint32_t i;

  // We're fitting an 8192 IQ samples to the display width.
  for (i = 0; i < bufferLength; i += 2)
  {
    points[i].x = (windowWidthInPixels / 2) + (short)signalBufferPtr[i];
    points[i].y = (windowHeightInPixels / 2) - (short)signalBufferPtr[i+1];
  } // for

  // Erase the previous plot.
  XClearWindow(displayPtr,window);

  // Make this display pretty.
  drawGridlines();

  // Set the signal color.
  XSetForeground(displayPtr,graphicsContext,scopeSignalColor);

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Annotate the display.  This is really too
  // sensitive to fonts.  I'll think of something
  // later.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  XDrawString(displayPtr,window,graphicsContext,
              annotationHorizontalPosition,
              annotationFirstLinePosition,
              sampleRateBuffer,strlen(sampleRateBuffer));

  XDrawString(displayPtr,window,graphicsContext,
              annotationHorizontalPosition,
              annotationSecondLinePosition,
              lissajousDivBuffer,strlen(lissajousDivBuffer));
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Plot the signal.
  XDrawPoints(displayPtr,
             window,
             graphicsContext,
             points,
             (bufferLength/2),
             CoordModeOrigin);

  // Send the request to the server
  XFlush(displayPtr);

  return;

} // plotLissajous

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
  // Each component is windowed so that sidelobes are
  // reduced.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  for (i = 0; i < bufferLength; i += 2)
  {
    // Store the real value.
    fftInputPtr[j][0] = (double)signalBufferPtr[i] * hanningWindow[j];

    // Store the imaginary value.
    fftInputPtr[j][1] = (double)signalBufferPtr[i+1] * hanningWindow[j];

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

    // We're reusing the magnitude buffer for power values.
    magnitudeBuffer[j] = (int16_t)powerInDb; 
  } // for
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (bufferLength / 2);

} // computePowerSpectrum


