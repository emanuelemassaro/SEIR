#ifndef NODE_HEADER_FILE_INCLUDED
#define NODE_HEADER_FILE_INCLUDED

#include <vector>

typedef struct Node
{
  std::vector<int> link;
  std::vector<double> force;
  std::vector<double> reducedForce;
  int m_n, m_oldN;
  int com;
  int k; // degree;

  int m_s, m_oldS; // susceptible
  int m_e, m_oldE; // exposed
  int m_i, m_oldI; // infected
  int m_r, m_oldR; // recovered
  
  bool m_countedSecondary;

  Node()
  {
    com = 0;
    k = 0;
    m_s = m_oldS = 0;
    m_i = m_oldI = 0;
    m_r = m_oldR = 0;
    m_e = m_oldE = 0;
    m_n = m_oldN = 0;
    m_countedSecondary = false;
  }

  inline void prepareOldStates()
  {
    m_oldS = m_s;
    m_oldI = m_i;
    m_oldR = m_r;
    m_oldE = m_e;
    m_oldN = m_n;
  }
} Node;

#endif // NODE_HEADER_FILE_INCLUDED
