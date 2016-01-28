#include <time.h>
#include <vector>

#include "model.h"
#include "utils.h"
#include "program.h"
#include "except.h"


Model::Model(double theta, double p,
             double rateSE, double rateEI, double rateIR,
             double gamma) :
  m_theta(theta), m_p(p), 
  m_rateSE(rateSE), m_rateEI(rateEI), m_rateIR(rateIR),
  m_gamma(gamma),
  m_nodes(0),
  m_eng((unsigned int) time(NULL)),
  m_secD(0)
{
}

Model::~Model()
{
  delete [] m_nodes;
}

void Model::getGraphFileName(int i, char *buffer)
{
#ifdef SF_NETWORKS
  sprintf(buffer, "%s/sf_%d/graph_%d.csv", Program::wdir(), DataPool::V, i);
#else
  sprintf(buffer, "%s/er_%d/graph_%d.csv", Program::wdir(), DataPool::V, i);
#endif
}

void Model::initNodes()
{
  m_nodes = new Node[DataPool::V];
 
  char fileName[MAXPATHLENGTH];
  getGraphFileName((int) (RAND_0_1 * 10), fileName);

  FILE *f = fopen(fileName, "r");
  auto_file a_f(f);

  while (!feof(f))
  {
    int i, j;
    fscanf(f, "%d,%d", &i, &j);
    m_nodes[i].link.push_back(j);
    m_nodes[i].k ++;
    m_nodes[j].link.push_back(i);
    m_nodes[j].k ++;
  }

  for (size_t i = 0; i < DataPool::V; i++)
  {
    Node &n1 = m_nodes[i];
    double totalTraffic = 0;
    for (int j = 0; j < n1.k; j++)
    {
      Node &n2 = m_nodes[j];
      totalTraffic += pow(n1.k * n2.k, m_theta); // total traffic in i
    }
    for (int j = 0; j < n1.k; j++)
    {
      Node &n2 = m_nodes[j];
      // force of interaction between i and j
      n1.force.push_back(m_p * pow(n1.k * n2.k, m_theta) / totalTraffic);
      // reduced force of interaction between i and j
      n1.reducedForce.push_back(m_p * m_gamma * pow(n1.k * n2.k, m_theta) / totalTraffic);
    }
    n1.m_n = DataPool::N;
  }

  int *prevNC = new int [DataPool::V];
  auto_del<int> del_prevNC(prevNC, true);

  long diff = DataPool::PC;
  long prevDiff = diff;
  while (diff <= prevDiff)
  {
    prevDiff = diff;

    for (size_t i = 0; i < DataPool::V; i++)
    {
      Node &n = m_nodes[i];
      n.prepareOldStates();
      prevNC[i] = n.m_n;
    }

    for (size_t i = 0; i < DataPool::V; i++)
    {
      Node &n = m_nodes[i];
      for (int j = 0; j < n.k; j++)
      {
        Node &n2 = m_nodes[n.link[j]];
        const double force = n.force[j];
      
        RandDistr distS((long) n.m_oldN, force);
        int tmp = distS(m_eng);
        n.m_n -= tmp;
        n.m_oldN -= tmp;
        n2.m_n += tmp;
      }
    }

    diff = 0;
    for (size_t i = 0; i < DataPool::V; i++)
      diff += abs(m_nodes[i].m_n - prevNC[i]);
  }
}

void Model::create()
{
  initNodes();

  const size_t ExpSP = 10;
  const size_t NumExp = DataPool::N / 100;

  std::vector<size_t> indexes;
  for (size_t i = 0; i < DataPool::V; i++)
    if (m_nodes[i].k == 2)
      indexes.push_back(i);

  if (indexes.size() < ExpSP)
    throw Exception("Insufficient number of nodes eligible for exposure");
  for (size_t i = 0; i < ExpSP; i++)
  {
    int r = rand() % indexes.size();
    m_nodes[indexes[r]].m_e = NumExp;
    m_nodes[indexes[r]].m_countedSecondary = true;
    indexes.erase(indexes.begin() + r);
  }
  for (int i = 0; i < DataPool::V; i++)
    m_nodes[i].m_s = DataPool::N - m_nodes[i].m_e;

  m_stepData.m_com = normalTraffic();
  m_stepData.m_comN = normalTraffic();
  m_stepData.m_sus = DataPool::PC - ExpSP * NumExp;
  m_stepData.m_exp = ExpSP * NumExp;
  m_stepData.m_inf = 0;
  m_stepData.m_rec = 0;
  m_stepData.m_expD = ExpSP;
  m_stepData.m_infD = 0;
  m_secD = 0;
}

void Model::advanceEpidemic()
{
  m_stepData.m_sus = 0;
  m_stepData.m_exp = 0;
  m_stepData.m_inf = 0;
  m_stepData.m_rec = 0;
  m_stepData.m_expD = 0;
  m_stepData.m_infD = 0;

  for (size_t i = 0; i < DataPool::V; i++)
  {
    Node &n = m_nodes[i];
    n.prepareOldStates();

    const double rse = (double) m_rateSE * n.m_oldI / n.m_oldN;
    const double rei = (double) m_rateEI;
    const double rir = (double) m_rateIR;

    RandDistr distSE((long) n.m_oldS, rse);
    int tmp = distSE(m_eng);
    n.m_s -= tmp;
    n.m_e += tmp;
    
    RandDistr distEI((long) n.m_oldE, rei);
    tmp = distEI(m_eng);
    n.m_e -= tmp;
    n.m_i += tmp;

    RandDistr distIR((long) n.m_oldI, rir);
    tmp = distIR(m_eng);
    n.m_i -= tmp;
    n.m_r += tmp;

    m_stepData.m_sus += n.m_s;
    m_stepData.m_exp += n.m_e;
    m_stepData.m_inf += n.m_i;
    m_stepData.m_rec += n.m_r;
    if (n.m_e > 0)
    {
      m_stepData.m_expD ++;
      if (!n.m_countedSecondary)
      {
        n.m_countedSecondary = true;
        m_secD ++;
      }
    }
    if (n.m_i > 0)
      m_stepData.m_infD ++;
  }
}

void Model::advanceTravel(double inf)
{
  for (size_t i = 0; i < DataPool::V; i++)
  {
    Node &n = m_nodes[i];
    n.prepareOldStates();
  }

  const double Threshold = 0;

  long com = 0;
  long comN = 0;

  for (size_t i = 0; i < DataPool::V; i++)
  {
    Node &n = m_nodes[i];

    int depS = 0, depSN = 0;
    int depE = 0, depEN = 0;
    int depI = 0, depIN = 0;
    int depR = 0, depRN = 0;

    const bool reduce = m_secD >= 5; // n.m_oldN > 0 && ((double) n.m_oldI / n.m_oldN) > Threshold;

    for (int j = 0; j < n.k; j++)
    {
      Node &n2 = m_nodes[n.link[j]];
      // reduce |= n2.m_oldN > 0 && ((double) n2.m_oldI / n2.m_oldN) > Threshold;
      // reduce &= (m_p != m_gamma);

      const double force = n.force[j];
      const double rForce = n.reducedForce[j];
            
      RandDistr distS((long) n.m_oldS, force);
      int tmp = distS(m_eng);
      if (n.m_oldS < tmp + depSN)
        tmp = n.m_oldS - depSN;
      depSN += tmp;
      comN += tmp;
      if (reduce)
      {
        RandDistr distS((long) n.m_oldS, rForce);
        tmp = distS(m_eng);
        if (n.m_oldS < tmp + depS)
          tmp = n.m_oldS - depS;
      }
      depS += tmp;
      com += tmp;
      n.m_s -= tmp;
      n2.m_s += tmp;


      RandDistr distE((long) n.m_oldE, force);
      tmp = distE(m_eng);
      if (n.m_oldE < tmp + depEN)
        tmp = n.m_oldE - depEN;
      depEN += tmp;
      comN += tmp;
      if (reduce)
      {
        RandDistr distSF((long) n.m_oldE, rForce);
        tmp = distE(m_eng);
        if (n.m_oldE < tmp + depE)
          tmp = n.m_oldE - depE;
      }
      depE += tmp;
      com += tmp;
      n.m_e -= tmp;
      n2.m_e += tmp;


	  RandDistr distI((long)n.m_oldI, force);
	  tmp = distI(m_eng);
	  comN += tmp;



      RandDistr distR((long) n.m_oldR, force);
      tmp = distR(m_eng);
      if (n.m_oldR < tmp + depRN)
        tmp = n.m_oldR - depRN;
      depRN += tmp;
      comN += tmp;
      if (reduce)
      {
        RandDistr distR((long) n.m_oldR, rForce);
        tmp = distR(m_eng);
        if (n.m_oldR < tmp + depR)
          tmp = n.m_oldR - depR;
      }
      depR += tmp;
      com += tmp;
      n.m_r -= tmp;
      n2.m_r += tmp;
    }
  }

  m_stepData.m_com = com;
  m_stepData.m_comN = comN;
}
