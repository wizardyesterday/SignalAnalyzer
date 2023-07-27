#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include <unistd.h>   // So we got the profile for 10 seconds

#include "SignalAnalyzer.h"

int main(int argc,char **argv)
{
  bool done;
  uint32_t i;
  uint32_t count;
  int8_t inputBuffer[16384];
  SignalAnalyzer *analyzerPtr;

  // Instantiate signal analyzer.
  analyzerPtr = new SignalAnalyzer(8192,280);

  // Set up for loop entry.
  done = false;

  while (!done)
  {
    // Read a 32 millisecond block of input samples.
    count = fread(inputBuffer,sizeof(int8_t),16384,stdin);

    if (count == 0)
    {
      // We're done.
      done = true;
    } // if
    else
    {
      analyzerPtr->plotSignalMagnitude(inputBuffer,count);
    } // else
  } // while

  // Release resources.
  delete analyzerPtr;

  return (0);

} // main
