#include "lib/cpp/io/input_std_istream.h"

#include <istream>

namespace handbag::io {
class NonOwningIstream {
public:
   explicit NonOwningIstream(std::istream& wrappee)
       : wrappee_(&wrappee) {}
private:
    std::istream* wrappee_ = nullptr;
};
}
