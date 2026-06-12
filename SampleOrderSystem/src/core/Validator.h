#pragma once
#include <string>

namespace Validator {

    inline bool isPositiveInt(int val) {
        return val >= 1;
    }

    inline bool isPositiveDouble(double val) {
        return val > 0.0;
    }

    inline bool isValidYield(double val) {
        return val > 0.0 && val <= 1.0;
    }

    inline bool isNonEmpty(const std::string& s) {
        return s.find_first_not_of(" \t\r\n") != std::string::npos;
    }

}
