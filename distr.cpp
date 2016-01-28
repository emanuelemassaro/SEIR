#include "distr.h"
#include "program.h"

#include <time.h>
#include <boost/random/uniform_01.hpp>

using namespace boost;
using namespace boost::random;



boost::mt19937 BoostBinDistr::m_eng;

void InitDistr(int rank)
{
  unsigned int seed = ((unsigned int) time(NULL)) * (rank + 1);
  srand(seed);
  Program::log("Random seeded with %X\n", seed);

  BoostBinDistr::m_eng.seed(((unsigned int) time(NULL)) * rand());
}




struct RandomGenerator
{
  RandomGenerator()
  {
    Program::log("Testing rand_0_1: program rank is %d\n", Program::mpiRank());
  }
};

double rand_0_1()
{
  static RandomGenerator rng;
  static mt19937 eng(((unsigned int) time(NULL)) * rand());
  static uniform_01<> dist;
  static variate_generator<mt19937 &, uniform_01<>> var(eng, dist);
  return var();
}

BoostBinDistr::BoostBinDistr(int n, double p) :
  m_dist(n, p),
  m_var(m_eng, m_dist)
{
}

