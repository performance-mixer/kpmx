#include <exception>
#include <cstdlib>

namespace boost {
[[noreturn]] void throw_exception(std::exception const &) {
  std::abort();
}
}
