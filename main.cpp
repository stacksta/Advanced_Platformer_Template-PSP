#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>

#include "callback.h"

extern "C" {
    #include "triGraphics.h"
    #include "triTypes.h"
    #include "triLog.h"
    #include "triMemory.h"
    #include "triTimer.h"
}

PSP_MODULE_INFO("PLATFORMER", 0x0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);


typedef struct Player {
    float x, y;
} Player;

int main(int argc, char *argv[])
{
    setupExitCallback();

    triLogInit();
    triMemoryInit();
    triInit(GU_PSM_8888, 1);

    Player player;
    player.x = 64.0f;
    player.y = 64.0f;

    float gravity = 100.0f;

    triTimer* deltaTime = triTimerCreate();

    while(isRunning())
    {
        triClear(0xFFFFFFFF);//color a b g r

        triTimerUpdate(deltaTime);


        player.y += gravity * triTimerPeekDeltaTime(deltaTime);

        triDrawRect(player.x, player.y, 10, 10, 0xff0000ff);

        triSwapbuffers();
    }

    triTimerFree(deltaTime);
    triClose();
    triMemoryCheck();
    triMemoryShutdown();

    sceKernelExitGame();

    return 0;
}