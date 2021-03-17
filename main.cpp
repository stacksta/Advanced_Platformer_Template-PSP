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
    float speed;
    float jumpHeight;
    bool onFloor;
    bool isMoving;
} Player;

typedef struct Floor {
    float x, y;
    float width, height;
} Floor;

int map[10][10] = { 
                    { 0, 0, 2, 0, 1, 0, 0, 0, 0, 0} , 
                    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} , 
                    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} ,                   
                    { 0, 0, 2, 0, 1, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0} ,  
                    { 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}          
                };

int main(int argc, char *argv[])
{
    SceCtrlData pad;

    setupExitCallback();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    triLogInit();
    triMemoryInit();
    triInit(GU_PSM_8888, 1);

    Player player { 64.0f, 64.0f, 10.0f, 10.0f, 50.0f, 2500.0f, false, false};

    float gravity = 100.0f;

    Floor floor { 64.0f, 100.0f, 500.0f, 30.0f};


    //load sprites
    triImage* playerSpriteIdle = triImageLoad("assets/sprites/king_Idle (78x58).png", 0);
    triImage* playerSpriteRun = triImageLoad("assets/sprites/king_Run (78x58).png" , 0);
    triImage* playerSpriteJump = triImageLoad("assets/sprites/king_Jump (78x58).png" , 0);

    triImage* terrainSpriteSheet = triImageLoad("assets/sprites/Terrain (32x32).png" , 0);

    //animations
    triImageAnimation* playerAnimationIdle = triImageAnimationFromSheet(playerSpriteIdle, 78.0f, 58.0f, 11, 1, 100);
    triImageAnimationStart(playerAnimationIdle);

    triImageAnimation* playerAnimationRun = triImageAnimationFromSheet(playerSpriteRun,  78.0f, 58.0f, 8, 1, 100);
    triImageAnimationStart(playerAnimationRun);


    triTimer* deltaTime = triTimerCreate();

    while(isRunning())
    {
        triClear(0xFFFFFFFF);//color a b g r

        triTimerUpdate(deltaTime);

        sceCtrlReadBufferPositive(&pad, 1);
        if(pad.Buttons != 0)
        {
            if(pad.Buttons & PSP_CTRL_RIGHT)
            {
                player.x += 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
            }
            else if(pad.Buttons & PSP_CTRL_LEFT)
            {
                player.x -= 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
            }
            if(pad.Buttons & PSP_CTRL_UP && player.onFloor)
                player.y -= 1.0f * player.jumpHeight * triTimerPeekDeltaTime(deltaTime);
        }
        if(pad.Buttons == 0)
            player.isMoving = false;


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



        for(int y = 0;y < 10;y++)
        {
            for(int x = 0;x < 10;x++)
            {
                if(map[y][x] == 1)
                    triDrawRect(x * 20, y * 20, 20.0f, 20.0f, 0xff00ffff);
                else if(map[y][x] == 2)
                    triDrawRect(x * 20, y * 20, 20.0f, 20.0f, 0xff0000ff);
            }
        }


        //draw floor
        triDrawRect(floor.x, floor.y, floor.width, floor.height, 0xff00ffff);
        triDrawImage(floor.x, floor.y, 32, 32, 64, 32, 96, 64, terrainSpriteSheet);


        //draw player
        triDrawRect(player.x, player.y, player.width, player.height, 0xff0000ff);

        if(!player.isMoving && player.onFloor)
        {
            triDrawImageAnimation(player.x - 78.0f/2.0f + player.width , player.y - 58.0f/2.0f, playerAnimationIdle);
            triImageAnimationUpdate(playerAnimationIdle);
        }
        else if(player.isMoving && player.onFloor)
        {
            triDrawImageAnimation(player.x - 78.0f/2.0f + player.width , player.y - 58.0f/2.0f, playerAnimationRun);
            triImageAnimationUpdate(playerAnimationRun); 
        }
        else 
        {
            triDrawSprite(player.x - 78.0f/2.0f + player.width , player.y - 58.0f/2.0f, 0, 0, playerSpriteJump);
        }

        triSwapbuffers();
    }

    triImageFree(playerSpriteIdle);
    triImageFree(playerSpriteRun);
    triImageFree(playerSpriteJump);
    triImageFree(terrainSpriteSheet);
    triTimerFree(deltaTime);
    triClose();
    triMemoryCheck();
    triMemoryShutdown();

    sceKernelExitGame();

    return 0;
}