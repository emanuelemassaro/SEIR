#ifndef MODEL_HEADER_INCLUDED
#define MODEL_HEADER_INCLUDED



#include "utils.h"
#include "node.h"
#include "datapool.h"
#include "stepdata.h"
#include "distr.h"

class Model
{
  double m_theta;
  double m_p;
  double m_rateSE;
  double m_rateEI;
  double m_rateIR;
  double m_gamma;

  Node *m_nodes;
  
  int m_totalInf;

  void getGraphFileName(int i, char *buffer);
  void initNodes();

  StepData m_stepData;
  size_t m_secD;

public:
  Model(double theta, double p,
        double rateSE, double rateEI, double rateIR,
        double gamma);
  virtual ~Model();

  void create();

  inline const StepData &stepData() const { return m_stepData; }

  inline size_t normalTraffic() const { return (size_t) (m_p * DataPool::PC); }

  void advanceEpidemic();
  void advanceTravel(double inf);
  void normalizeNodes(int i);
};

#endif // MODEL_HEADER_INCLUDED
