#include <iostream>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <ctime>
#include <string>

class RandStringGenerator {
public:
  typedef boost::minstd_rand gen_type;

  static std::string generate(unsigned _length) {
    boost::uniform_int<> range(0, allowed.size());
    boost::variate_generator<gen_type&, boost::uniform_int<> > gen_idx(rng, range);
  
    std::string sid;
    for (unsigned i = 0; i < _length; ++i) {
      sid += allowed[gen_idx() % allowed.size()];
    }
    return sid;
  }

private:
  static std::string allowed;
  static gen_type rng;
};

std::string RandStringGenerator::allowed = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
RandStringGenerator::gen_type RandStringGenerator::rng(std::time(0));

