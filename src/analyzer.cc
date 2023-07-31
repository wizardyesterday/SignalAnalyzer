//*************************************************************************
// File name: analyzer.cc
//*************************************************************************

//*************************************************************************
// This program provides the ability to display either the magnitude of
// IQ (In-phase or Quadrature) signals that are provided by stdin.  The
// data is 8-bit signed 2's complement, and is formatted as
// I1,Q1; I2,Q2; ...
//
// To run this program type,
// 
//     ./analyzer > -d displaytype -f frequency -r sampleRate
//                -d duration > ncoFileName,
//
// where,
//
//    amplitude - 1 for magnitude display and 2 for power spectrum display.
//    filename - Tne input source.
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
  // Default 1/2 scale.
  *parameters.displayTypePtr = SignalMagnitude;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"d:h");

    switch (opt)
    {
      case 'd':
      {
        *parameters.displayTypePtr = atoi(optarg);
        break;
      } // case

      case 'h':
      {
        // Display usage.
        fprintf(stderr,"./analyzer -d [1 - magnitude | 2 - spectrum] \n");

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
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.displayTypePtr = &displayType;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Instantiate signal analyzer.
  analyzerPtr = new SignalAnalyzer((DisplayType)displayType,1024,256);

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

    } // else
  } // while

  // Release resources.
  delete analyzerPtr;

  return (0);

} // main
