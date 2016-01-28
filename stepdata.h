#ifndef STEPDATA_HEADER_FILE
#define STEPDATA_HEADER_FILE

#include <cmath>
#include <cstring>

typedef struct StepData
{
  long m_com;
  long m_comN;
  long m_sus;
  long m_exp;
  long m_inf;
  long m_rec;
  long m_expD;
  long m_infD;

  StepData()
  {
    reset();
  }

  StepData(const StepData &v) :
    m_com(v.m_com), m_comN(v.m_comN), m_sus(v.m_sus),
    m_exp(v.m_exp), m_inf(v.m_inf), m_rec(v.m_rec),
    m_expD(v.m_expD), m_infD(v.m_infD)
  {
  }

  void reset()
  {
    memset(this, 0, sizeof(struct StepData));
  }

  StepData &operator= (const StepData &v)
  {
    memcpy(this, &v, sizeof(*this));
    return *this;
  }

  void operator+= (const StepData &v)
  {
    m_com += v.m_com;
    m_comN += v.m_comN;
    m_sus += v.m_sus;
    m_exp += v.m_exp;
    m_inf += v.m_inf;
    m_rec += v.m_rec;
    m_expD += v.m_expD;
    m_infD += v.m_infD;
  }
  void operator-= (const StepData &v)
  {
    m_com -= v.m_com;
    m_comN -= v.m_comN;
    m_sus -= v.m_sus;
    m_exp -= v.m_exp;
    m_inf -= v.m_inf;
    m_rec -= v.m_rec;
    m_expD -= v.m_expD;
    m_infD -= v.m_infD;
  }
} StepData;

#endif // STEPDATA_HEADER_FILE
