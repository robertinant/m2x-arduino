#ifndef NullPrint_h
#define NullPrint_h

#include "Print.h"

class NullPrint : public Print {
public:
  virtual size_t write(uint8_t b) {
    return 1;
  }
};

#endif  /* NullPrint_h */
