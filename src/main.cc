#include "charfix.h"
#include <windows.h>

#include "globals.h"
#include "overlay.h"
#include "console.h"
#include "utils.h"
#include "activation/activation.h"
#include "vmprotect.h"
//#include "activation/activation.h"

GlobalVars* g_Vars;
Driver* g_Drv;

void OfflineOffsets() 
{
    ProtectStart();
    
    // updated: 4/11/2020
    g_Vars->offsets.lastupdate = 1586597346;
    
    g_Vars->offsets.localPlayer = 0x1dfe9d8; // updated
    g_Vars->offsets.entityList = 0x1897f38; // updated
    g_Vars->offsets.punchAngle = 0x2308; // updated
    g_Vars->offsets.viewMatrix = 0x1b3bd0; // updated
    g_Vars->offsets.viewRender = 0xcb0da70; // updated
    g_Vars->offsets.studioHdr = 0x10e0;

    g_Vars->offsets.viewAngles = 0x23D0; // updated
    g_Vars->offsets.cameraPos = 0x1DA8; // updated
    g_Vars->offsets.vecOrigin = 0x14C; // updated
    g_Vars->offsets.absVelocity = 0x140;
    g_Vars->offsets.activeWeapon = 0x1944;
    g_Vars->offsets.boneClass = 0xee0; // updated
    g_Vars->offsets.propName = 0x518; // updated
    g_Vars->offsets.bleedout = 0x2590; 
    g_Vars->offsets.lifeState = 0x730;
    g_Vars->offsets.teamNum = 0x3f0; // updated
    g_Vars->offsets.health = 0x3e0; // updated
    g_Vars->offsets.shield = 0x170; // updated
    g_Vars->offsets.flags = 0x98;

    g_Vars->offsets.bulletSpeed = 0x1D2C;
    g_Vars->offsets.bulletGravity = 0x1d34;

    ProtectEnd();
}

void OfflineSettings() 
{
    ProtectStart();
    
    g_Vars->settings.maxfps = 60;
    
    g_Vars->settings.visuals.enabled = true;
    g_Vars->settings.visuals.box = true;
    g_Vars->settings.visuals.health = true;
    g_Vars->settings.visuals.shield = true;
    g_Vars->settings.visuals.showTarget = true;
    g_Vars->settings.visuals.fovCircle = false;
    g_Vars->settings.visuals.hideTeammates = false;
    
    g_Vars->settings.aim.enabled = true;
    g_Vars->settings.aim.aimkey = VK_XBUTTON2;
    g_Vars->settings.aim.maxfov = 10.0f;
    g_Vars->settings.aim.nopunch = true;
    g_Vars->settings.aim.maxdistance = 5000;
    
    g_Vars->settings.aim.smooth = true;
    g_Vars->settings.aim.divider = 200;
    
    g_Vars->settings.aim.gravity = true;
    g_Vars->settings.aim.velocity = true;
    
    g_Vars->settings.aim.teamCheck = true;
    g_Vars->settings.aim.knockCheck = true;

    ProtectEnd();
}

void CheckProcess(void* blank) 
{
    while (true) 
    {
        if (Utils::FindProcess(EW(L"r5apex.exe")) == 0) 
        {
            Console::WriteLog("Game is not running anymore!");
            g_Vars->shouldExit = true;
            return;
        }
        Sleep(1000);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{            
    ProtectStart();
    
    g_Vars = new GlobalVars();
    g_Vars->activated = false;
    g_Vars->shouldExit = false;
    g_Vars->ready = false;

    OfflineOffsets();
    OfflineSettings();

#ifdef TESTBUILD
    Console::WriteLog(E("Starting in debug mode..."));
    Console::WriteLog(E("Allocating and connecting to system console..."));
    AllocConsole();
    freopen_s((FILE**)stdin,  E("CONIN$"), "r",  stdin);
    freopen_s((FILE**)stdout, E("CONOUT$"), "w", stdout); 
    printf(E("GVAR size: %i"), sizeof(GlobalVars));
    
    g_Vars->activated = true;
#else
    g_Vars->activated = Activation::Activate();
#endif
    
    Console::WriteLog(E("Creating threads..."));
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Overlay::Loop, nullptr, 0, nullptr);
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)Console::DisplayLoop, nullptr, 0, nullptr);
    CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CheckProcess, nullptr, 0, nullptr);

    Console::WriteLog(E("Getting program info..."));
    int pid = Utils::FindProcess(EW(L"r5apex.exe"));
    Console::WriteLog(E("Game PID is %i"), pid);
    uintptr_t baseaddr = Utils::GetBase(pid, E("r5apex.exe"));
    Console::WriteLog(E("Game base address is %llx"), baseaddr);
    g_Vars->apexBase = baseaddr;

    if (!pid || !baseaddr) 
    {
        g_Vars->shouldExit = true;
        Console::WriteLog(E("Failed to get process information"));
    }
    
    if (!g_Vars->shouldExit) 
    {
        Console::WriteLog(E("Trying to connect to the driver..."));
        g_Drv = new Driver();
        g_Drv->Init(pid);   
        int test = g_Drv->Read<int>(g_Vars->apexBase);
        Console::WriteLog(E("Test read: %i"), test);

        Console::WriteLog(E("Using offsets from %s"), Utils::UnixDate(g_Vars->offsets.lastupdate).c_str());
        g_Vars->ready = true;
    }

    while (!g_Vars->shouldExit) 
    {
        Sleep(1000);
    }

    g_Vars->ready = false;
    Console::WriteLog(E("Exit signal received. Exiting."));
    Sleep(5000);

    return 0;

    ProtectEnd();
}