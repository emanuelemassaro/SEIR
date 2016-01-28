#ifndef PROGRAM_HEADER_FILE_INCLUDED
#define PROGRAM_HEADER_FILE_INCLUDED

#include "datapool.h"
#include "utils.h"

#include <stdio.h>

class Program
{
public:
  static void log(const char *txt, ...);

  static const char *wdir() { return sm_wdir; }

private:
  static DataPool sm_dp;

  static int sm_mpiRank;

  static FILE *m_logFile;
  static void initLog();
  static void deinitLog();

  static char sm_wdir[MAXPATHLENGTH];

  static void calcIndex(int ix);

  static bool parseArgs(int argc, char *argv[]);
  static bool runMainMPI();
  static void runWorkerMPI();

  static void preparseCFMeans();

public:
  static int run(int argc, char *argv[]);
  static int mpiRank() { return sm_mpiRank; }
};

#endif // PROGRAM_HEADER_FILE_INCLUDED
