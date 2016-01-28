#ifndef DATA_POOL_HEADER_FILE
#define DATA_POOL_HEADER_FILE

#include "stepdata.h"

#include <stdio.h>
#include <vector>

class Model;

class DataPool
{
  struct IndexData
  {
    bool m3;

    double Theta;
    double P;
    double R0;
    double RateEI;
    double RateIR;
    double Gamma;

    int sample;
  };

  bool *m_ready;
  bool *m_dirsReady;
  std::vector<StepData> *m_data;
  int m_indexCount;

  const IndexData &indexData(int i);
  bool haveFileForIndex(int i);

  const char *dir1Path(const IndexData &data);
  const char *dir2Path(const IndexData &data);
  const char *runPath(const IndexData &data);

  int m_startIndex;

public:
  static const size_t V = 5000;
  static const size_t N = 5000;
  static const size_t PC = V * N;

  static const int NUM_TRIALS = 10;

  DataPool();
  virtual ~DataPool();

  bool checkFS();

  bool createDirs(int ix);
  FILE *openSampleFile(int ix, const char *mode);

  inline void resetIndex() { m_startIndex = 0; }
  int nextIndex();

  Model *newModel(int index);
  void setIndex(int index, bool v);
};

#endif // DATA_POOL_HEADER_FILE
