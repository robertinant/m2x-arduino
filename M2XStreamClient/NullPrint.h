#ifndef NullPrint_h
#define NullPrint_h

#include "Print.h"

// Null Print class used to calculate length to print
class NullPrint : public Print {
public:
  virtual size_t write(uint8_t b) {
    return 1;
  }
};

#endif  /* NullPrint_h */
