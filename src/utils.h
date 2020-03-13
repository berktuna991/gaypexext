#ifndef UTILS_H
#define UTILS_H
#include <string>

namespace Utils 
{
    std::string UnixDate(int date);
    int FindProcess(const wchar_t* proc);
    uintptr_t GetBase(int pid, const char* modulename);
    void LimitFPS(int targetfps);
}

#endif