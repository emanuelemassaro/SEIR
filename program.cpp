#include <string.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include <mpi.h>
#include "mpimsg.h"

#include "program.h"
#include "except.h"
#include "utils.h"
#include "datapool.h"
#include "model.h"

#ifndef WIN32
#  define _stricmp strcasecmp
#endif

using namespace std;

FILE *Program::m_logFile = NULL;
DataPool Program::sm_dp;
char Program::sm_wdir[MAXPATHLENGTH];
int Program::sm_mpiRank = -1;

bool Program::parseArgs(int argc, char *argv[])
{
  if (argc < 1)
    return false;
  strcpy(sm_wdir, argv[1]);
  if (!FS::fsEntryExist(sm_wdir))
    return false;
  return true;
}

void Program::initLog()
{
  char path[MAXPATHLENGTH];
  
  sprintf(path, "%s%c%s%c%d.log",
          wdir(), FS::pathSep(),
          "epidnetlog", FS::pathSep(),
          sm_mpiRank);
  m_logFile = fopen(path, "wb");
}

void Program::log(const char *txt, ...)
{
  if (m_logFile != NULL)
  {
    static char buffer[1024];

    va_list aptr;
    va_start(aptr, txt);
    vsprintf(buffer, txt, aptr);
    va_end(aptr);

    fprintf(m_logFile, "%s", buffer);
    fflush(m_logFile);
  }
}

void Program::deinitLog()
{
  if (m_logFile != NULL)
  {
    fclose(m_logFile);
    m_logFile = NULL;
  }
}

void Program::calcIndex(int ix)
{
  vector<StepData> sampleData;

  log("Starting IX %d\n", ix);

  Model *m = sm_dp.newModel(ix);
  auto_del<Model> del_m(m, false);

  m->create();

  const StepData *stepData = &m->stepData();
  sampleData.push_back(*stepData);
  while (sampleData.size() < 10000)
  { 
    m->advanceEpidemic();
    m->advanceTravel(stepData->m_inf);
    stepData = &m->stepData();
    if (stepData->m_inf == 0)
    {
      StepData tmp(*stepData);
      tmp.m_com = m->normalTraffic();
      tmp.m_comN = m->normalTraffic();
      sampleData.push_back(tmp);
      break;
    }
    else
      sampleData.push_back(*stepData);
  }

  if (FILE *outFile = sm_dp.openSampleFile(ix, "wb"))
  {
    auto_file close_outFile(outFile);

    fprintf(outFile, "\"Step\",\"Com\",\"ComN\",\"S\",\"E\",\"I\",\"R\",\"ExpD\",\"InfD\"\n");

    for (size_t i = 0; i < sampleData.size(); i++)
    {
      StepData &sd = sampleData[i];
      fprintf(outFile, "\"%d\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\",\"%ld\"\n",
              i, sd.m_com, sd.m_comN, sd.m_sus, sd.m_exp, sd.m_inf, sd.m_rec, sd.m_expD, sd.m_infD);
    }
  }
  else
  {
    char txt[100];
    sprintf(txt, "Can't open file for index %d", ix);
    throw Exception(txt);
  }
}

bool Program::runMainMPI()
{
  int mpiSize;
  MPI_Status s;
  MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);

  bool ready = sm_dp.checkFS();

  log("Main ready state is %s\n", ready ? "true" : "false");

  if (ready)
  {
    int msg = MSG(MSG_QUIT, 0);
    for (int i = 1; i < mpiSize; i++)
    {
      MPI_Recv(&msg, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
      MPI_Bsend(&msg, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
  }
  else
  {
    sm_dp.resetIndex();

    int procsFinished = 0;
    while (procsFinished < mpiSize - 1)
    {
      int msg;
      MPI_Recv(&msg, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
    
      int src = s.MPI_SOURCE;
      if (src > 0 && src < mpiSize)
      {
        int cmd = MSG_CMD(msg);

        if (cmd == WMSG_READY)
        {
          log("Process %d is ready\n", src);

          int ix = sm_dp.nextIndex();
          if (ix < 0 || !sm_dp.createDirs(ix))
          {
            int msg = MSG(MSG_QUIT, 0);
            MPI_Bsend(&msg, 1, MPI_INT, src, 0, MPI_COMM_WORLD);
            procsFinished ++;
          }
          else
          {
            int msg = MSG(MSG_RUN, ix);
            MPI_Bsend(&msg, 1, MPI_INT, src, 0, MPI_COMM_WORLD);
          }
        }
      }
      else
        break;
    }

    ready = mpiSize > 1;
  }

  return ready;
}

void Program::runWorkerMPI()
{
  while (true)
  {
    MPI_Status s;
    int msg = MSG(WMSG_READY, 0);
    MPI_Bsend(&msg, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
    MPI_Recv(&msg, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &s);

    int cmd = MSG_CMD(msg);
    if (cmd != MSG_RUN)
      return;
    int ix = MSG_DATA(msg);
    calcIndex(ix);
  }
}

int Program::run(int argc, char *argv[])
{
  if (!parseArgs(argc, argv))
    return 1;

  int retCode = 0;

  int size = 4096;
  char buffer[4096];

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &sm_mpiRank);
  MPI_Buffer_attach(buffer, size);

  if (sm_mpiRank != 0)
  {
    int tmp = 0;
    MPI_Status s;
    MPI_Recv(&tmp, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
  }
  else
  {
    int mpiSize, tmp = 1;
    MPI_Comm_size(MPI_COMM_WORLD, &mpiSize);
    FS::makeDir(wdir(), "epidnetlog");
    for (int i = 1; i < mpiSize; i++)
      MPI_Bsend(&tmp, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
  }
  
  initLog(); // call after MPI startup routines to have a rank

  InitDistr(sm_mpiRank + 1);
  log("Our rank is %d\n", sm_mpiRank);

  try
  {
    if (sm_mpiRank == 0)
      runMainMPI();
    else
      runWorkerMPI();
  }
  catch (const Exception &e)
  {
    log("Fatal exception: %s\n", e.message());
    retCode = 1;
  }

  MPI_Buffer_detach(buffer, &size);
  MPI_Finalize();

  deinitLog();

  return retCode;
}
