#include "datapool.h"
#include "program.h"
#include "except.h"
#include "utils.h"
#include "model.h"

using namespace std;

static double Theta_Pool[] = { 0.5 };
static double P_Pool[] = { 1.0 };  // mobility scale
static double R0_Pool[] =  { 2.0 };
static double RateEI_Pool[] = { 0.3 };
static double RateIR_Pool[] =  { 0.1 };
static const double Gamma_Pool[] = { 0.000001, 0.00001, 0.0001, 0.001, 0.01, 0.1, 1.0 };

static int Theta_C = sizeof(Theta_Pool) / sizeof(*Theta_Pool);
static int P_C = sizeof(P_Pool) / sizeof(*P_Pool);
static int R0_C = sizeof(R0_Pool) / sizeof(*R0_Pool);
static int RateEI_C = sizeof(RateEI_Pool) / sizeof(*RateEI_Pool);
static int RateIR_C = sizeof(RateIR_Pool) / sizeof(*RateIR_Pool);
static int Gamma_C = sizeof(Gamma_Pool) / sizeof(*Gamma_Pool);


DataPool::DataPool()
{
  m_indexCount = Theta_C * P_C * R0_C *
                 RateEI_C * RateIR_C * Gamma_C * NUM_TRIALS;
  m_ready = NULL;
  m_data = NULL;
  m_dirsReady = NULL;
}

DataPool::~DataPool()
{
  delete [] m_dirsReady;
  delete [] m_ready;
  delete [] m_data;
}

const DataPool::IndexData &DataPool::indexData(int i)
{
  static IndexData ret;

  int tmp = i;

  ret.sample = tmp % NUM_TRIALS;
  tmp /= NUM_TRIALS;

  ret.Gamma = Gamma_Pool[tmp % Gamma_C];
  tmp /= Gamma_C;

  ret.R0 = R0_Pool[tmp % R0_C];
  tmp /= R0_C;

  ret.RateEI = RateEI_Pool[tmp % RateEI_C];
  tmp /= RateEI_C;
  ret.RateIR = RateIR_Pool[tmp % RateIR_C];

  tmp /= RateIR_C;
  
  ret.P = P_Pool[tmp % P_C];
  tmp /= P_C;

  ret.Theta = Theta_Pool[tmp % Theta_C];
  tmp /= Theta_C;

  return ret;
}

const char *DataPool::dir1Path(const IndexData &data)
{
  static char buffer[MAXPATHLENGTH];

#ifdef SF_NETWORKS
  sprintf(buffer, "sf");
#else
  sprintf(buffer, "er");
#endif
  return buffer;
}

const char *DataPool::dir2Path(const IndexData &data)
{
  static char ret[MAXPATHLENGTH];
  sprintf(ret, "data_%.4lf_%.1lf_%.1lf_%.1lf_%lf",
          data.P, data.R0, data.RateEI, data.RateIR, data.Gamma);
  return ret;
}

const char *DataPool::runPath(const IndexData &data)
{
  static char ret[MAXPATHLENGTH];

  sprintf(ret, "run_%d.csv",
          data.sample);
  return ret;
}

bool DataPool::haveFileForIndex(int i)
{
  const char *wdir = Program::wdir();

  const IndexData &data = indexData(i);

  char fullPath[MAXPATHLENGTH];

  const char *d1P = dir1Path(data);
  const char ps = FS::pathSep();

  sprintf(fullPath, "%s%c%s%c%s%c%s",
          wdir, ps, d1P, ps, dir2Path(data), ps, runPath(data));

  return FS::fsEntryExist(fullPath);
}

bool DataPool::checkFS()
{
  if (m_ready == NULL)
    m_ready = new bool [m_indexCount];

  bool ret = true;

  for (int i = 0; i < m_indexCount; i++)
    ret &= (m_ready[i] = haveFileForIndex(i));

  return ret;
}

Model *DataPool::newModel(int ix)
{
  const IndexData &data = indexData(ix);

  Model *model = new Model(data.Theta, data.P,
                           data.RateIR * data.R0, data.RateEI, data.RateIR,
                           data.Gamma);

  Program::log("Created model for (%lf) #%d\n",
               data.Gamma, data.sample);

  return model;
}

bool DataPool::createDirs(int ix)
{
  if (m_dirsReady == NULL)
  {
    m_dirsReady = new bool [m_indexCount / NUM_TRIALS];
    memset(m_dirsReady, 0, m_indexCount * sizeof(bool) / NUM_TRIALS);
  }
  int dirIx = ix / NUM_TRIALS;
  if (m_dirsReady[dirIx])
    return true;

  const char *wdir = Program::wdir();

  const IndexData &data = indexData(ix);
  
  char dirPath[MAXPATHLENGTH];

  const char *d1P = dir1Path(data);
  const char *d2P = dir2Path(data);
  const char ps = FS::pathSep();
  
  sprintf(dirPath, "%s%c%s", d1P, ps, d2P);

  m_dirsReady[dirIx] = FS::makeDir(wdir, dirPath);
  return m_dirsReady[dirIx];
}

FILE *DataPool::openSampleFile(int ix, const char *mode)
{
  const char *wdir = Program::wdir();

  const IndexData &data = indexData(ix);

  const char *d1P = dir1Path(data);
  const char *d2P = dir2Path(data);
  const char ps = FS::pathSep();

  char fullPath[MAXPATHLENGTH];
  sprintf(fullPath, "%s%c%s%c%s%c%s", wdir, ps,
          d1P, ps, d2P, ps, runPath(data));

  return fopen(fullPath, mode);
}

int DataPool::nextIndex()
{
  while (m_startIndex < m_indexCount)
  {
    if (!m_ready[m_startIndex])
    {
      int r = m_startIndex;
      m_startIndex ++;
      return r;
    }
    m_startIndex ++;
  }
  return -1;
}

