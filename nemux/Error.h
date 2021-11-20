#ifndef ERROR_H_
#define ERROR_H_

#include <stdexcept>

struct invalid_format : public std::runtime_error {
    explicit invalid_format(const std::string & what)
        : std::runtime_error(what)
    {}

    explicit invalid_format(const char * what)
        : std::runtime_error(what)
    {}
};

struct unsupported_format : public std::runtime_error {
    explicit unsupported_format(const std::string & what)
        : std::runtime_error(what)
    {}

    explicit unsupported_format(const char * what)
        : std::runtime_error(what)
    {}
};

#endif //ERROR_H_
