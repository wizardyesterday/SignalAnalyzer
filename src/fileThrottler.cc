//*************************************************************************
// File name: fileThrottler.cc
//*************************************************************************

//*************************************************************************
// This program provides reads from stdin at a specified block size (in
// bytes), and allows delay between reads, of a specified delay in
// microseconds.  In other words, the output is throttled.
// As an example, my signal anzlyzer reads its input from stdin.  If IQ
// data has been captured to a file at a sample rate of 256000S/s, and
// 16384 bytes were in each block of data, a 32ms (32000 us) delay would
// be specified.  This allows a realtime playback of the data.  This
// data could be piped to my signal analyzer app for realtime playback
// of the file (as it were being streamed from my rtlsdr diags).
//
// To run this program type,
// 
//     ./fileThrottler > -b blockSize -d <delayTime>
//
// where,
//
//    blockSize - The number of bytes in each block that is read.
//
//    delayTime - Delay time, in microseconds, between reads from stdin.
///*************************************************************************
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_BLOCK_SIZE (65536)
#define DEFAULT_BLOCK_SIZE (16384)
#define DEFAULT_DELAY (32000)

// This structure is used to consolidate user parameters.
struct MyParameters
{
  uint32_t *blockSizePtr;
  uint32_t *delayPtr;
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
  // Default number of bytes to read.
  *parameters.blockSizePtr = DEFAULT_BLOCK_SIZE;

  // Default to 32ms (delay in microseconds).
  *parameters.delayPtr = DEFAULT_DELAY;
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/

  // Set up for loop entry.
  done = false;

  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  // Retrieve the command line arguments.
  //_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
  while (!done)
  {
    // Retrieve the next option.
    opt = getopt(argc,argv,"b:d:h");

    switch (opt)
    {
      case 'b':
      {
        *parameters.blockSizePtr = atol(optarg);

        if (*parameters.blockSizePtr > MAX_BLOCK_SIZE)
        {
          // Keep it sane.
          *parameters.blockSizePtr = MAX_BLOCK_SIZE;
        } // if
        break;
      } // case

      case 'd':
      {
        *parameters.delayPtr = atol(optarg);
        break;
      } // case

      case 'h':
      {
        // Display usage.
        fprintf(stderr,"./fileThrottler -b blockSizeInBytes "
                "-d delayTimeInMicrosedonds\n");

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
  uint32_t count;
  int status;
  int8_t inputBuffer[MAX_BLOCK_SIZE];
  uint32_t blockSize;
  uint32_t delay;
  struct MyParameters parameters;

  // Set up for parameter transmission.
  parameters.blockSizePtr = &blockSize;
  parameters.delayPtr = &delay;

  // Retrieve the system parameters.
  exitProgram = getUserArguments(argc,argv,parameters);

  if (exitProgram)
  {
    // Bail out.
    return (0);
  } // if

  // Set up for loop entry.
  done = false;

  while (!done)
  {
    // Read a block of input samples (2 * complex FFT length).
    count = fread(inputBuffer,1,blockSize,stdin);

    if (count == 0)
    {
      // We're done.
      done = true;
    } // if
    else
    {
      // Write to stdout to pipe to another program.
      fwrite(inputBuffer,1,blockSize,stdout);

      // Throttle the output.
      status = usleep(delay);
    } // else

  } // while

  return (0);

} // main
