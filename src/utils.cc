#include "charfix.h"
#include <string>
#include <stdio.h>
#include <time.h>
#include <stdint.h>
#include <iomanip>
#include <sstream>
#include <windows.h>
#include <winternl.h>
#include <tlhelp32.h>
#include "utils.h"
#include "legacy.h"
#include "console.h"
#include "vmprotect.h"

std::string Utils::UnixDate(int date) 
{
    std::uint32_t time_date_stamp = date;
    time_t temp = time_date_stamp;
    std::tm* t = gmtime(&temp);
    std::stringstream ss;
    ss << std::put_time(t, "%Y-%m-%d %I:%M:%S %p");
    std::string output = ss.str();
    return output;
}

int Utils::FindProcess(const wchar_t* proc)
{
	ProtectStart();
    
    auto snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	auto pe = PROCESSENTRY32{ sizeof(PROCESSENTRY32) };

	if (Process32First(snapshot, &pe)) {
		do {
			if (wcscmp(proc, pe.szExeFile) == 0) {
				CloseHandle(snapshot);
				return pe.th32ProcessID;
			}
		} while (Process32Next(snapshot, &pe));
	}
	CloseHandle(snapshot);
	return 0;

    ProtectEnd();
}

std::wstring Utils::ToWideChar(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

uintptr_t Utils::GetBase(int pid, const char* modulename) 
{
    ProtectStart();
    
    ModuleInfo mi = GetProcessModules(pid);
    for (int i = 0; i < MODULE_MAX; i++) 
    {
        ModuleDll mod = mi.list[i];
        if (!mod.name) continue;
        
        if (strstr(mod.name, modulename)) 
        {         
            return mod.baseaddress;
        }
    }
    return 0;

    ProtectEnd();
}

static DWORD LastFrameTime = 0;
void Utils::LimitFPS(int targetfps) 
{
    DWORD currentTime = timeGetTime();
    if ( (currentTime - LastFrameTime) < (1000 / targetfps))
    {
        Sleep(currentTime - LastFrameTime);
    }
    LastFrameTime = currentTime;   
}

void Utils::RandomText(char *s, const int len) 
{
    ProtectStart();
    
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;

    ProtectEnd();
}