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
    float width, height;
    bool onFloor;
} Player;

typedef struct Floor {
    float x, y;
    float width, height;
} Floor;

int main(int argc, char *argv[])
{
    SceCtrlData pad;

    setupExitCallback();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    triLogInit();
    triMemoryInit();
    triInit(GU_PSM_8888, 1);

    Player player { 64.0f, 64.0f, 10.0f, 10.0f, false};

    float gravity = 100.0f;

    Floor floor { 64.0f, 100.0f, 100.0f, 30.0f};

    triTimer* deltaTime = triTimerCreate();

    while(isRunning())
    {
        triClear(0xFFFFFFFF);//color a b g r

        triTimerUpdate(deltaTime);

        sceCtrlReadBufferPositive(&pad, 1);
        if(pad.Buttons != 0)
        {
            if(pad.Buttons & PSP_CTRL_RIGHT)
                player.x += 1.0f;
            if(pad.Buttons & PSP_CTRL_LEFT)
                player.x -= 1.0f;
        }

        if(player.x < floor.x + floor.width &&
            player.x + player.width > floor.x &&
            player.y < floor.y + floor.height &&
            player.y + player.height > floor.y) 
        {
            // floor detected
            player.onFloor = true;
        }
        else 
            player.onFloor = false;


        if(!player.onFloor)
            player.y += gravity * triTimerPeekDeltaTime(deltaTime);


        //draw floor
        triDrawRect(floor.x, floor.y, floor.width, floor.height, 0xff00ffff);

        //draw player
        triDrawRect(player.x, player.y, player.width, player.height, 0xff0000ff);

        triSwapbuffers();
    }

    triTimerFree(deltaTime);
    triClose();
    triMemoryCheck();
    triMemoryShutdown();

    sceKernelExitGame();

    return 0;
}