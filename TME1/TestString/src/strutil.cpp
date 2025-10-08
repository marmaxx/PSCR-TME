// strutil.cpp
#include "strutil.h"

namespace pr {

size_t length(const char* s) {
    size_t len = 0;
    for (size_t i = 0; s[i]; ++i) {
        ++len;
    }
    return len;
}

char* newcopy(const char* s) {
    size_t len = length(s);
    char* copy = new char[len + 1];
    for (size_t i = 0; i <= len; ++i) {
        copy[i] = s[i];
    }
    return copy;
}

int compare(const char* a, const char* b) {
    if (length(a) != length(b)) return -1;
    else {
        for (size_t i = 0; a[i]; ++i) {
            if (a[i] != b[i]) return -1;
        }
    }
    return 0;
}

}
