#ifndef TIME_CLOCK_H_
#define TIME_CLOCK_H_

#include <time.h>

class TimeClock {
protected:
  double start_;
  double stop_;
  
public:
  void reset() {
    start_ = stop_ = 0;
  }
  
  void start() { 
    start_ = clock(); 
  }
  
  void stop() { 
    stop_ = clock(); 
  }
  
  double time() const {
    return ((double)(stop_ - start_))/(CLOCKS_PER_SEC/1000); 
  }
};



#endif /*TIME_CLOCK_H_*/
