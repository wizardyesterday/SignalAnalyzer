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
//     ./analyzer > -d displaytype -D
//
// where,
//
//    displayType - The type of display.  Valid values are;
//    1 - Magnitude display.
//    2 - Power spectrum display.
//
//    The D flag indicates that raw IQ data should be dumped to stdout.
//    This allows the data to be piped to another program.  Here's how
//    to do this (for example, using a spectral display):
//    ./analyzer -d 2 > >(other program to accept IQ data)
///*************************************************************************
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <unistd.h>   // So we got the profile for 10 seconds

#include "SignalAnalyzer.h"

// This structure is used to consolidate user parameters.
struct MyParameters
{
  int *displayTypePtr;
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
    opt = getopt(argc,argv,"d:-Dh");

    switch (opt)
    {
      case 'd':
      {
        *parameters.displayTypePtr = atoi(optarg);
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
        fprintf(stderr,"./analyzer -d [1 - magnitude | 2 - spectrum] "
                " -D (dump raw IQ)\n");

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
  bool exitProgram;
  uint32_t i;
  uint32_t count;
  int8_t inputBuffer[16384];
  SignalAnalyzer *analyzerPtr;
  int displayType;
  bool iqDump;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.displayTypePtr = &displayType;
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
                                   1024,
                                   256,
                                   256000);

  // Set up for loop entry.
  done = false;

  while (!done)
  {
    // Read a 32 millisecond block of input samples.
    count = fread(inputBuffer,sizeof(int8_t),(2 * N),stdin);

    if (count == 0)
    {
      // We're done.
      done = true;
    } // if
    else
    {
      switch (displayType)
      {
        case SignalMagnitude:
        {
          analyzerPtr->plotSignalMagnitude(inputBuffer,count);
          break;
        } // case

        case PowerSpectrum:
        {
          analyzerPtr->plotPowerSpectrum(inputBuffer,count);
          break;
        } // case

      } // switch

      if (iqDump == true)
      {
        // Write to stdout so that raw IQ can be piped to another program.
        fwrite(inputBuffer,sizeof(int8_t),(2 * N),stdout);
      } // if

    } // else
  } // while

  // Release resources.
  delete analyzerPtr;

  return (0);

} // main
