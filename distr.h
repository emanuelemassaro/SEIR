#include <random>

#ifndef DISTR_HEADER_FILE_INCLUDED
#define DISTR_HEADER_FILE_INCLUDED


#include <boost/version.hpp>
#include <boost/random.hpp>
#include <boost/random/binomial_distribution.hpp>

#define RAND_0_1 rand_0_1()
double rand_0_1();



void InitDistr(int seed);


class IDegreeDistr
{
public:
  virtual int gen() = 0;
};

class BoostBinDistr : public IDegreeDistr
{
  friend void InitDistr(int seed);

  static boost::mt19937 m_eng;

#if BOOST_VERSION < 105700
  boost::binomial_distribution<> m_dist;
  boost::variate_generator<boost::mt19937&, boost::binomial_distribution<>> m_var;
#else
  boost::random::binomial_distribution<> m_dist;
  boost::variate_generator<boost::mt19937&, boost::random::binomial_distribution<>> m_var;
#endif
  
public:
  BoostBinDistr(int n, double p);
  inline int gen() { return m_var(); }
};

#endif // DISTR_HEADER_FILE_INCLUDED
