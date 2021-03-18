#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>

#include <vector>

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
    bool isJump;
    bool isRight;
} Player;

// typedef struct Floor {
//     float x, y;
//     float width, height;
// } Floor;

const int ROW = 10;
const int COL = 15;

int map[ROW][COL] = { 
                    { 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5} , 
                    { 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6} , 
                    { 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6} ,
                    { 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6} ,
                    { 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6} ,
                    { 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6} ,                   
                    {10, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8} ,
                    { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3} , //floor
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,  
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}          
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

    Player player { 64.0f, 64.0f, 10.0f, 10.0f, 50.0f, 400.0f, false, false, false, true};

    float gravity = 100.0f;

    //Floor floor { 0, 224.0f, 500.0f, 30.0f};

    std::vector<std::pair<int, int>> floorTiles;
    for(int i=0;i < ROW * COL;i++)
    {
        int x = i / COL;
        int y = i % ROW;
        if(map[y][x] == 3)
            floorTiles.push_back(std::make_pair(x, y));
    }


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
        triClear(0xFF51383F);//color a b g r

        triTimerUpdate(deltaTime);

        sceCtrlReadBufferPositive(&pad, 1);

        if(pad.Buttons != 0)
        {
            if(pad.Buttons & PSP_CTRL_RIGHT)
            {
                player.x += 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
                player.isRight = true;
            }
            else if(pad.Buttons & PSP_CTRL_LEFT)
            {
                player.x -= 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
                player.isRight = false;
            }
            if(pad.Buttons & PSP_CTRL_UP && player.onFloor)
            {
                player.isJump = true;
                player.onFloor = false;
            }
        }
        if(pad.Buttons == 0)
            player.isMoving = false;

        if(player.isJump)
        {
            player.y -= 1.0f * player.jumpHeight * triTimerPeekDeltaTime(deltaTime);
            player.jumpHeight -= 10.0f;
        }

        for(int i = 0;i < floorTiles.size(); i++)
        {
            int x = floorTiles[i].first * 32;
            int y = floorTiles[i].second * 32;
            int width = x + 32;
            int height = y + 32;

            if(player.x < x + width &&
                player.x + player.width > x &&
                player.y < y + height &&
                player.y + player.height > y) 
            {            
                // floor detected
                player.onFloor = true;
                player.isJump = false;
                player.jumpHeight = 400.0f;

                player.y -= 3.0f;
            } 
        }

        if(!player.onFloor)
            player.y += gravity * triTimerPeekDeltaTime(deltaTime);



        for(int y = 0;y < ROW;y++)
        {
            for(int x = 0;x < COL;x++)
            {
                if(map[y][x] == 1)
                    triDrawImage(x * 32, y * 32, 32, 32, 64, 256, 96, 288, terrainSpriteSheet);
                else if(map[y][x] == 2)
                    triDrawImage(x * 32, y * 32, 32, 32, 64, 224, 96, 256, terrainSpriteSheet); //draw wall background
                else if(map[y][x] == 3)
                    triDrawImage(x * 32, y * 32, 32, 32, 64, 32, 96, 64, terrainSpriteSheet); //draw floor
                else if(map[y][x] == 4)
                    triDrawImage(x * 32, y * 32, 32, 32, 32, 224, 64, 256, terrainSpriteSheet);
                else if(map[y][x] == 5)
                    triDrawImage(x * 32, y * 32, 32, 32, 96, 224, 128, 256, terrainSpriteSheet);
                else if(map[y][x] == 6)
                    triDrawImage(x * 32, y * 32, 32, 32, 96, 256, 128, 288, terrainSpriteSheet);
                else if(map[y][x] == 7)
                    triDrawImage(x * 32, y * 32, 32, 32, 64, 288, 96, 320, terrainSpriteSheet);
                else if(map[y][x] == 8)
                    triDrawImage(x * 32, y * 32, 32, 32, 96, 288, 128, 320, terrainSpriteSheet);
                else if(map[y][x] == 9)
                    triDrawImage(x * 32, y * 32, 32, 32, 32, 256, 64, 288, terrainSpriteSheet);
                else if(map[y][x] == 10)
                    triDrawImage(x * 32, y * 32, 32, 32, 32, 288, 64, 320, terrainSpriteSheet);
            }
        }

        //draw floor
        //triDrawRect(floorTiles[0].first * 32, floorTiles[0].second * 32, 32, 32, 0xff00ffff);
        //triDrawImage(floor.x, floor.y, 32, 32, 64, 32, 96, 64, terrainSpriteSheet);


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