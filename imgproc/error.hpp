#ifndef imgproc_error_hpp_included_
#define imgproc_error_hpp_included_

#include <stdexcept>
#include <string>

namespace imgproc {

#define DECLARE_EXCEPTION(type, base) \
    struct type : public base { type(const std::string &msg) : base(msg) {} }

DECLARE_EXCEPTION(Error, std::runtime_error);
DECLARE_EXCEPTION(TypeError, Error);

#undef DECLARE_EXCEPTION

} // namespace imgproc

#endif // imgproc_error_hpp_included_
