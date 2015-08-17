#include "utils.h"

void splitString(const char *str, char ch, vector<string> &out)
{
    const char *ptr;
    while (ptr = strchr(str, ch)) {
        out.push_back(string(str, ptr - str));
        str = ptr + 1;
    }
    if (*str) {
        out.push_back(string(str));
    }
}
