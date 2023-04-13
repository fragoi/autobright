#include "logger.h"

using namespace std;

ostream *Logger::out = &cout;
ostream *Logger::err = &cerr;

ostream& operator<<(ostream &out, const exception_ptr &ptr) {
  const char *msg;
  try {
    rethrow_exception(ptr);
  } catch (const exception &e) {
    msg = e.what();
  } catch (...) {
    msg = "unknown";
  }
  return out << msg;
}
