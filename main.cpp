#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>

#include <psptypes.h>
#include <time.h>
#include<psprtc.h>

#include <math.h>
#include <bits/stdc++.h>
#include <vector>

#include "callback.h"
#include "objects.h"
#include "collision.h"

extern "C"
{
#include "triGraphics.h"
#include "triTypes.h"
#include "triLog.h"
#include "triMemory.h"
#include "triTimer.h"
#include "triVAlloc.h"
#include "triFont.h"
}

PSP_MODULE_INFO("PLATFORMER", 0x0, 1, 1);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);

/*from libpspmath*/
float vfpu_sqrtf(float x)
{
    float result;
    __asm__ volatile(
        "mtv     %1, S000\n"
        "vsqrt.s S000, S000\n"
        "mfv     %0, S000\n"
        : "=r"(result)
        : "r"(x));
    return result;
}


const int ROW = 10;
const int COL = 25;

int map[ROW][COL] = {
        {11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13, 0},    
        {11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,14},
        {11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,14},
        {11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,14},
        {11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,14},
        {11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,14},
        {11, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,12, 3, 3, 3,15, 1, 1, 1, 1,14},
        {12, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,17, 0, 0, 0, 0,15, 1, 1, 1,14}, //floor
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

int decor[ROW][COL] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 2, 0, 5, 6, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 3, 4, 0, 7, 8, 0, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //floor
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };



float getDistance(float x1, float y1, float x2, float y2)
{
    float a = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
    a = vfpu_sqrtf(a);
    if (isnan(a) == 0)
        return a;
    else
        return 0.0f;
}

int main(int argc, char *argv[])
{
    SceCtrlData pad;

    setupExitCallback();

    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);

    triFontInit();
    triLogInit();
    triMemoryInit();
    triInit(GU_PSM_8888, 1);

    Player player{ 480.0f / 2.0f, 272.0f / 2.0f, 10.0f, 10.0f, 50.0f, 400.0f, 3, 3, false, false, false, true, false};
    Collider playerFeet{player.x, player.y, 5.0f, 5.0f};
    Collider playerRight{player.x, player.y, 5.0f, 5.0f};
    Collider playerLeft{player.x, player.y, 5.0f, 5.0f};

    Enemy en1{ 480.0f / 2.0f + 64.0f, 272.0f / 2.0f, 10.0f, 10.0f, 10.0f, false, false, false, false, false};
    Collider enFeet1{ en1.x, en1.y, 5.0f, 5.0f};
    Collider enLeft1{ en1.x, en1.y, 5.0f, 5.0f};
    Collider enRight1{ en1.x, en1.y, 5.0f, 5.0f};

    Camera camera{0, 0};

    float gravity = 100.0f;

    Floor floor{0, 224.0f, 480.0f, 32.0f};
    Floor floor1{480.0f, 192.0f, 160.0f, 32.0f};
    Floor floor2{640.0f, 224.0f, 32.0f, 32.0f};
    Floor floor3{672.0f, 256.0f, 128.0f, 32.0f};

    Floor leftWall{0, 0, 32.0f, 256.0f};
    Floor rightWall{768.0f, 0, 32.0f, 288.0f};

    //load sprites  
    triImage *playerSpriteIdle = triImageLoad("assets/sprites/player_Idle (32x32).png", 0);
    triImage *playerSpriteRun = triImageLoad("assets/sprites/player_Run (32x32).png", 0);
    triImage *playerSpriteJump = triImageLoad("assets/sprites/player_Jump (32x32).png", 0);
    triImage *playerSpriteHeart = triImageLoad("assets/sprites/big_Heart_Idle (18x14).png", 0);

    triImage *terrainSpriteSheet = triImageLoad("assets/sprites/terrain.png", 0);
    // triImage *decorSpriteSheet = triImageLoad("assets/sprites/Decorations (32x32).png", 0);

    triImage *enemySpriteIdle = triImageLoad("assets/sprites/mushroom_Idle (32x32).png", 0);
    triImage *enemySpriteRun = triImageLoad("assets/sprites/mushroom_Run (32x32).png", 0);

    // triImage *doorSprite = triImageLoad("assets/sprites/door_Idle.png", 0);

    // //animations
    triImageAnimation *playerAnimationIdle = triImageAnimationFromSheet(playerSpriteIdle, 32.0f, 32.0f, 11, 1, 100);
    triImageAnimationStart(playerAnimationIdle);

    triImageAnimation *playerAnimationRun = triImageAnimationFromSheet(playerSpriteRun, 32.0f, 32.0f, 12, 1, 100);
    triImageAnimationStart(playerAnimationRun);

    triImageAnimation *playerAnimationHeart = triImageAnimationFromSheet(playerSpriteHeart, 18.0f, 14.0f, 8, 1, 100);
    triImageAnimationStart(playerAnimationHeart);

    // triImageAnimation *enemyAnimationIdle = triImageAnimationFromSheet(enemySpriteIdle, 34.0f, 28.0f, 11, 1, 100);
    // triImageAnimationStart(enemyAnimationIdle);

    triImageAnimation *enemyAnimationRun = triImageAnimationFromSheet(enemySpriteRun, 32.0f, 32.0f, 16, 1, 100);
    triImageAnimationStart(enemyAnimationRun);

    triTimer *deltaTime = triTimerCreate();

    //bool test = false;

    while (isRunning())
    {
        triClear(0x301F21); //color a b g r

        triTimerUpdate(deltaTime);

        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons != 0)
        {
            if (pad.Buttons & PSP_CTRL_RIGHT)
            {
                //player.x += 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
                player.isRight = true;

                camera.x -= 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
            }
            else if (pad.Buttons & PSP_CTRL_LEFT)
            {
                //player.x -= 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
                player.isRight = false;

                camera.x += 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
            }
            if (pad.Buttons & PSP_CTRL_UP && player.onFloor)
            {
                player.isJump = true;
                player.onFloor = false;
            }
        }
        if (pad.Buttons == 0)
        {
            player.isMoving = false;
            //player.isAttack = false;
        }

        playerFeet.x = player.x + 10.0f;
        playerFeet.y = player.y + 10.0f;

        playerRight.x = player.x + 15.0f;
        playerRight.y = player.y;

        playerLeft.x = player.x;
        playerLeft.y = player.y;

        if (player.isJump)
        {
            //player.y -= 1.0f * player.jumpHeight * triTimerPeekDeltaTime(deltaTime);
            player.jumpHeight -= 10.0f;

            camera.y += 1.0f * player.jumpHeight * triTimerPeekDeltaTime(deltaTime);
        }

        //floor collision
        if (checkPlayerFloorCollision(&playerFeet, &floor, &camera) || checkPlayerFloorCollision(&playerFeet, &floor1, &camera) || checkPlayerFloorCollision(&playerFeet, &floor2, &camera) || checkPlayerFloorCollision(&playerFeet, &floor3, &camera))
        {
            // floor detected
            player.onFloor = true;
            player.isJump = false;
            player.jumpHeight = 400.0f;

            //player.y -= 3.0f;
            //camera.y += 3.0f;
        }
        else
        {
            player.onFloor = false;
            player.isAttack = false;
        }

        if (!player.onFloor)
        {
            //player.y += gravity * triTimerPeekDeltaTime(deltaTime);
            camera.y -= gravity * triTimerPeekDeltaTime(deltaTime);
        }
        //wall collision
        if (checkPlayerFloorCollision(&playerRight, &floor1, &camera) || checkPlayerFloorCollision(&playerRight, &floor2, &camera) || checkPlayerFloorCollision(&playerRight, &rightWall, &camera))
        {
            camera.x += 1.0f;
        }
        if (checkPlayerFloorCollision(&playerLeft, &floor1, &camera) || checkPlayerFloorCollision(&playerLeft, &floor2, &camera) || checkPlayerFloorCollision(&playerLeft, &leftWall, &camera))
        {
            camera.x -= 1.0f;
        }

        enFeet1.x = en1.x;
        enFeet1.y = en1.y + 10.f;

        enLeft1.x = en1.x - 10.f;
        enLeft1.y = en1.y;

        enRight1.x = en1.x + 10.f;
        enRight1.y = en1.y;

        if(checkCollision(&enFeet1, &floor, &camera))
        {
            en1.onFloor = true;
        }
        else
            en1.onFloor = false;

        if (!en1.onFloor)
        {
            en1.y += gravity * triTimerPeekDeltaTime(deltaTime);
        }
        if(checkCollision(&enRight1, &floor1, &camera))
        {
            en1.x -= 1.0f;
        }

        /*ai*/
        float distance = getDistance(player.x, player.y, en1.x + camera.x, en1.y + camera.y);
        if(!en1.isDead)
        {
            if (distance < 150.0f)
            {
                if (player.x < en1.x + camera.x)
                {
                    en1.x -= 1.0f * en1.speed * triTimerPeekDeltaTime(deltaTime);
                    en1.isMoving = true;
                    //en1.isAttack = false;
                }
                else if (player.x > en1.x + camera.x)
                {
                    en1.x += 1.0f * en1.speed * triTimerPeekDeltaTime(deltaTime);
                    en1.isMoving = true;
                    //en1.isAttack = false;
                }
                else
                {
                    en1.x = en1.x;
                    en1.isMoving = false;
                }
            }
            else
            {
                en1.isMoving = false;
            }
        }

        if(checkPlayerEnemyCollision(&playerFeet, &en1, &camera) && !en1.isDead)
        {
            if(player.isJump)
            {
                player.isAttack = true;
                en1.isDead = true;
                player.isJump = true;
                player.onFloor = false;
                player.jumpHeight = 420.0f;
            }
        }
        if(checkPlayerCollision(&playerRight, &enLeft1, &camera) && !en1.isDead)
        {
            camera.x += 1.0f * 1000.0f * triTimerPeekDeltaTime(deltaTime);
            player.isJump = true;
            player.onFloor = false;
            player.jumpHeight = 250.0f;
            player.health--;
        }
        if(checkPlayerCollision(&playerLeft, &enRight1, &camera) && !en1.isDead)
        {
            camera.x -= 1.0f * 1000.0f * triTimerPeekDeltaTime(deltaTime);
            player.isJump = true;
            player.onFloor = false;
            player.jumpHeight = 250.0f;
            player.health--;
        }
       
        for (int y = 0; y < ROW; y++)
        {
            for (int x = 0; x < COL; x++)
            {
                if(map[y][x] == 1)//bbb8d0
                    triDrawRect(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 0xffd0b8bb);
                else if(map[y][x] == 3)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 16, 0, 32, 16, terrainSpriteSheet);
                else if(map[y][x] == 12)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 0, 0, 16, 16, terrainSpriteSheet);
                else if(map[y][x] == 15)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 32, 0, 48, 16, terrainSpriteSheet);
                else if(map[y][x] == 11)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 32, 16, 48, 32, terrainSpriteSheet);
                else if(map[y][x] == 14)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 0, 16, 16, 32, terrainSpriteSheet);

                //draw decor
                /*if (decor[y][x] == 1)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 64, 96, 96, 128, decorSpriteSheet);
                */
            }
        }
        
        //draw floor
        //triDrawRect(floor.x + camera.x, floor.y + camera.y, floor.width, floor.height, 0xff00ffff);
        //triDrawRect(floor1.x + camera.x, floor1.y + camera.y, floor1.width, floor1.height, 0xff00ffff);
        //triDrawRect(floor2.x + camera.x, floor2.y + camera.y, floor2.width, floor2.height, 0xff00ffff);
        //triDrawRect(floor3.x + camera.x, floor3.y + camera.y, floor3.width, floor3.height, 0xff00ffff);


        //triDrawRect(leftWall.x + camera.x, leftWall.y + camera.y, leftWall.width, leftWall.height, 0xff00ffff);
        //triDrawRect(rightWall.x + camera.x, rightWall.y + camera.y, rightWall.width, rightWall.height, 0xff00ffff);

        //draw door
        //triDrawSprite(80.0f + camera.x, 272.0f / 2.0f + 32.0f + camera.y, 0, 0, doorSprite);

        //draw enemy
        //triDrawRect(en1.x + camera.x, en1.y + camera.y, en1.width, en1.height, 0xff0000ff);


        //draw HUD
        if(player.health == 3)
        {
            triDrawImageAnimation(46, 10, playerAnimationHeart);
            triImageAnimationUpdate(playerAnimationHeart);
        }
        if(player.health >= 2)
        {
            triDrawImageAnimation(28, 10, playerAnimationHeart);
            triImageAnimationUpdate(playerAnimationHeart);
        }
        triDrawImageAnimation(10, 10, playerAnimationHeart);
        triImageAnimationUpdate(playerAnimationHeart);
        
        if (!en1.isMoving && !en1.isDead)
        {
            //triDrawImageAnimation(en1.x - 32.0f / 2.0f + en1.width - 10.0f + camera.x, en1.y - 32.0f / 2.0f + camera.y, enemyAnimationIdle);
            //triImageAnimationUpdate(enemyAnimationIdle);
            triDrawSprite(en1.x - 32.0f / 2.0f + en1.width - 10.0f + camera.x, en1.y - 32.0f / 2.0f + camera.y, 0, 0, enemySpriteIdle);
        }
        else if (en1.isMoving && !en1.isDead)
        {
            triDrawImageAnimation(en1.x - 32.0f / 2.0f + en1.width - 10.0f + camera.x, en1.y - 32.0f / 2.0f + camera.y, enemyAnimationRun);
            triImageAnimationUpdate(enemyAnimationRun);
        }

        //triDrawImage(floor.x, floor.y, 32, 32, 64, 32, 96, 64, terrainSpriteSheet);

        //draw player
        //triDrawRect(player.x, player.y, player.width, player.height, 0xff0000ff);
        
        
        if (!player.isMoving && player.onFloor)
        {
            triDrawImageAnimation(player.x - 32.0f / 2.0f + player.width, player.y - 32.0f / 2.0f, playerAnimationIdle);
            triImageAnimationUpdate(playerAnimationIdle);
        }
        else if (player.isMoving && player.onFloor)
        {
            triDrawImageAnimation(player.x - 32.0f / 2.0f + player.width, player.y - 32.0f / 2.0f, playerAnimationRun);
            triImageAnimationUpdate(playerAnimationRun);
        }
        else
        {
            triDrawSprite(player.x - 32.0f / 2.0f + player.width, player.y - 32.0f / 2.0f, 0, 0, playerSpriteJump);
        }

/*
        triDrawRect(playerLeft.x, playerLeft.y, playerLeft.width, playerLeft.height, 0xff0000ff);
        triDrawRect(playerRight.x, playerRight.y, playerRight.width, playerRight.height, 0xff0000ff);
        triDrawRect(playerFeet.x, playerFeet.y, playerFeet.width, playerFeet.height, 0xff0000ff);

        triDrawRect(enLeft1.x + camera.x, enLeft1.y + camera.y, enLeft1.width, enLeft1.height, 0xff0000ff);
        triDrawRect(enRight1.x + camera.x, enRight1.y + camera.y, enRight1.width, enRight1.height, 0xff0000ff);
        triDrawRect(enFeet1.x + camera.x, enFeet1.y + camera.y, enFeet1.width, enFeet1.height, 0xff0000ff);
*/

 /*       triFontActivate(0);
        triFontPrintf(0, 0, 0xFFFFFFFF, "FPS: %.2f - MAX: %.2f - MIN: %.2f", triFps(), triFpsMax(), triFpsMin());
        triFontPrintf(0, 10, 0xFFFFFFFF, "CPU: %.2f%% - GPU: %.2f%%", triCPULoad(), triGPULoad());
        triFontPrintf(0, 20, 0xFFFFFFFF, "Player: x = %.2f , y = %.2f", player.x - camera.x, player.y - camera.y);
        triFontPrintf(0, 30, 0xFFFFFFFF, "onFloor: %d", player.onFloor);
        triFontPrintf(0, 40, 0xFFFFFFFF, "Camera: x = %.2f , y = %.2f", camera.x, camera.y);
        triFontPrintf(0, 50, 0xFFFFFFFF, "Distance: %.2f", getDistance(player.x, player.y, en1.x + camera.x, en1.y + camera.y));
        triFontPrintf(0, 60, 0xFFFFFFFF, "Health: %d", player.health);
*/

        triSwapbuffers();
    }

    triImageFree(enemySpriteIdle);
    triImageFree(enemySpriteRun);
    triImageFree(playerSpriteIdle);
    triImageFree(playerSpriteRun);
    triImageFree(playerSpriteJump);
    triImageFree(playerSpriteHeart);
    /*triImageFree(playerSpriteAttack);
    triImageFree(decorSpriteSheet);*/
    triImageFree(terrainSpriteSheet);
    /*triImageFree(doorSprite);*/
    triTimerFree(deltaTime);
    triClose();
    triMemoryCheck();
    triMemoryShutdown();

    sceKernelExitGame();

    return 0;
}