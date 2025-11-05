//*************************************************************************
// File name: analyzer.cc
//*************************************************************************

//*************************************************************************
// This program provides the ability to display either the magnitude of
// IQ (In-phase or Quadrature) signals that are provided by stdin.  The
// data is 8-bit signed 2's complement, and is formatted as
// I1,Q1; I2,Q2; ...
// This program can also pass raw IQ data to stdout if required.
//
// To run this program type,
// 
//    ./analyzer -d <displaytype> -r <sampleRate> -V <verticalgain>
//              -R <referenceLevel -U -D < inputFile
//
// where,
//
//    displayType - The type of display.  Valid values are;
//    1 - Magnitude display.
//    2 - Power spectrum display.
//    3 - Lissajous display.
//
//    he R flag sets the reference level on the spectrum analyzer display.
//
//    The V flag sets the vertical gain of the signal yo be displayed on
//    the spectrum analyzer display.
//
//    The U flag indicates that the IQ samples are unsigned 8-bit
//    quantities rather than the default signed values.  This allows
//    this program to work with the standard rtl-sdr tools such as
//    rtl_sdr.
//
//    The D flag indicates that raw IQ data should be dumped to stdout.
//    This allows the data to be piped to another program.  Here's how
//    to do this (for example, using a spectral display):
//    ./analyzer -d 2 > >(other program to accept IQ data).
//
//    sampleRate - The sample rate of the IQ data in S/s.
//
//    referenceLevel - The reference level of the spectrum display in dB.
//
///*************************************************************************
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "SignalAnalyzer.h"

// This structure is used to consolidate user parameters.
struct MyParameters
{
  int *displayTypePtr;
  float *sampleRatePtr;
  float *verticalGainPtr;
  int32_t *spectrumReferenceLevelPtr;
  bool *unsignedSamplesPtr;
  bool *iqDumpPtr;
};

/*****************************************************************************

  Name: getUserArguments

  Purpose: The purpose of this function is to retrieve the user arguments
  that were passed to the program.  Any arguments that are specified are
  set to reasonable default values.

  Calling Sequence: exitProgram = getUserArguments(parameters)

  Inputs:

    parameters - A structure that contains pointers to the user parameters.

  Outputs:

    exitProgram - A flag that indicates whether or not the program should
    be exited.  A value of true indicates to exit the program, and a value
    of false indicates that the program should not be exited..

*****************************************************************************/
bool getUserArguments(int argc,char **argv,struct MyParameters parameters)
{
  bool exitProgram;
  bool done;
  int opt;

  // Default not to exit program.
  exitProgram = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default parameters.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Default oscilloscope display.
  *parameters.displayTypePtr = SignalMagnitude;

  // Default to 256000S/s.
  *parameters.sampleRatePtr = 256000;

  // Default to no amplification.
  *parameters.verticalGainPtr = 1;

  // Default to 0dB reference level.
  *parameters.spectrumReferenceLevelPtr = 0;

  // Default to signed IQ samples.
  *parameters.unsignedSamplesPtr = false;

  // Default to not dumping IQ data.
  *parameters.iqDumpPtr = false;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"d:r:V:R:UDh");

    switch (opt)
    {
      case 'd':
      {
        *parameters.displayTypePtr = atoi(optarg);
        break;
      } // case

      case 'r':
      {
        *parameters.sampleRatePtr = atof(optarg);
        break;
      } // case

      case 'V':
      {
        *parameters.verticalGainPtr = atof(optarg);
        break;
      } // case

      case 'R':
      {
        *parameters.spectrumReferenceLevelPtr = atol(optarg);
        break;
      } // case

      case 'U':
      {
        *parameters.unsignedSamplesPtr = true;
        break;
      } // case

      case 'D':
      {
        *parameters.iqDumpPtr = true;
        break;
      } // case

      case 'h':
      {
        // Display usage.
        fprintf(stderr,"./analyzer -d [1 - magnitude | 2 - spectrum |"
                " 3 - lissajous]\n"
                "           -r samplerate (S/s) \n"
                "           -R spectrumreferencelevel (dB)\n"
                "           -V Vertical gain of signal to display\n"
                "           -U (unsigned samples)\n"
                "           -D (dump raw IQ) < inputFile\n");

        // Indicate that program must be exited.
        exitProgram = true;
        break;
      } // case

      case -1:
      {
        // All options consumed, so bail out.
        done = true;
      } // case
    } // switch

  } // while
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  return (exitProgram);

} // getUserArguments

//*************************************************************************
// Mainline code.
//*************************************************************************
int main(int argc,char **argv)
{
  bool done;
  int8_t *signedBufferPtr;
  bool exitProgram;
  uint32_t i;
  uint32_t count;
  uint8_t inputBuffer[16384];
  SignalAnalyzer *analyzerPtr;
  int displayType;
  float sampleRate;
  bool unsignedSamples;
  float verticalGain;
  int32_t spectrumReferenceLevel;
  bool iqDump;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.displayTypePtr = &displayType;
  parameters.sampleRatePtr = &sampleRate;
  parameters.unsignedSamplesPtr = &unsignedSamples;
  parameters.verticalGainPtr = &verticalGain;
  parameters.spectrumReferenceLevelPtr = &spectrumReferenceLevel;
  parameters.iqDumpPtr = &iqDump;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Instantiate signal analyzer.
  analyzerPtr = new SignalAnalyzer((DisplayType)displayType,
                                   sampleRate,
                                   verticalGain,
                                   spectrumReferenceLevel);

  // Reference the input buffer in 8-bit signed context.
  signedBufferPtr = (int8_t *)inputBuffer;

  // Set up for loop entry.
  done = false;

  while (!done)
  {
    // Read a block of input samples (2 * complex FFT length).
    count = fread(inputBuffer,sizeof(int8_t),(2 * N),stdin);

    if (count == 0)
    {
      // We're done.
      done = true;
    } // if
    else
    {
      if (unsignedSamples)
      {
        for (i = 0; i < count; i++)
        {
          // Convert unsigned samples to signed quantities.
          signedBufferPtr[i] -= 128;
        } // for
      } // if

      switch (displayType)
      {
        case SignalMagnitude:
        {
          analyzerPtr->plotSignalMagnitude(signedBufferPtr,count);
          break;
        } // case

        case PowerSpectrum:
        {
          analyzerPtr->plotPowerSpectrum(signedBufferPtr,count);
          break;
        } // case

        case Lissajous:
        {
          analyzerPtr->plotLissajous(signedBufferPtr,count);
          break;
        } // case

      } // switch

      if (iqDump == true)
      {
        // Write to stdout so that raw IQ can be piped to another program.
        fwrite(signedBufferPtr,sizeof(int8_t),(2 * N),stdout);
      } // if

    } // else
  } // while

  // Release resources.
  delete analyzerPtr;

  return (0);

} // main
