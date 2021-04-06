#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>

#include <math.h>
#include <vector>

#include "callback.h"
#include "objects.h"
#include "collision.h"
#include "camera.h"

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
const int COL = 32;

int map[ROW][COL] = {
        {22,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,22, 0, 0},    
        {11, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,14, 0, 0},
        {11, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1,14, 0, 0},
        {11, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,14, 0, 0},
        {11, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 7, 9, 2, 1, 2, 1, 2, 1, 2, 1,14, 0, 0},
        {11, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2,10,17, 1, 2, 1, 2, 1, 2, 1, 2,14, 0, 0},
        {11, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 7, 8, 8, 8, 9,17, 2, 1, 2, 1, 2, 1, 2, 1,14, 0, 0},
        {22, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,15,10,16,16,16,17,17, 1, 2, 1, 2, 1, 2, 1, 2,14, 0, 0}, //floor
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,11,18,19,19,19,20,12, 3, 3, 3, 3, 3, 3, 3, 3, 22, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,22, 3, 3, 3, 3, 3,22, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
    };

int decor[ROW][COL] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 1, 2, 0, 5, 6, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 3, 4, 0, 7, 8, 0, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //floor
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
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

    Player player{ 480.0f / 2.0f, 272.0f / 2.0f, 10.0f, 10.0f, 100.0f, 400.0f, 3, 3, false, false, false, true, false};
    Collider playerFeet{player.x, player.y, 5.0f, 5.0f};
    Collider playerRight{player.x, player.y, 5.0f, 5.0f};
    Collider playerLeft{player.x, player.y, 5.0f, 5.0f};

    //player.speed = 30.0f;

    Enemy en1{ 480.0f / 2.0f + 64.0f, 272.0f / 2.0f, 10.0f, 10.0f, 10.0f, false, false, false, false, false};
    en1.feet.x = en1.x; en1.feet.y = en1.y; en1.feet.width = en1.feet.height = 5.0f;
    en1.colliderLeft.x = en1.x; en1.colliderLeft.y = en1.y; en1.colliderLeft.width = en1.colliderLeft.height = 5.0f;
    en1.colliderRight.x = en1.x; en1.colliderRight.y = en1.y; en1.colliderRight.width = en1.colliderRight.height = 5.0f;

    std::vector<Enemy> enemyPool;
    enemyPool.push_back(en1);

    Camera camera = Camera(0, 0, COL * 32, ROW * 32);

    float gravity = 100.0f;

    Floor floor{0, 224.0f, 480.0f, 32.0f};
    Floor floor1{480.0f, 192.0f, 160.0f, 64.0f};
    Floor floor2{608.0f, 128.0f, 64.0f, 32.0f};
    Floor floor3{640.0f, 256.0f, 288.0f, 32.0f};

    Floor leftWall{0, 0, 32.0f, 256.0f};
    Floor rightWall{928.0f, 0, 32.0f, 288.0f};

    //load sprites  
    triImage *playerSpriteIdle = triImageLoad("assets/sprites/player_Idle (32x32).png", 0);
    triImage *playerSpriteRun = triImageLoad("assets/sprites/player_Run (32x32).png", 0);
    triImage *playerSpriteJump = triImageLoad("assets/sprites/player_Jump (32x32).png", 0);
    triImage *playerSpriteHeart = triImageLoad("assets/sprites/big_Heart_Idle (18x14).png", 0);

    triImage *terrainSpriteSheet = triImageLoad("assets/sprites/terrain.png", 0);
    // triImage *decorSpriteSheet = triImageLoad("assets/sprites/Decorations (32x32).png", 0);

    triImage *enemySpriteIdle = triImageLoad("assets/sprites/mushroom_Idle (32x32).png", 0);
    triImage *enemySpriteRun = triImageLoad("assets/sprites/mushroom_Run (32x32).png", 0);

    triImage *sawSprite = triImageLoad("assets/sprites/saw_On (38x38).png", 0);

    // triImage *doorSprite = triImageLoad("assets/sprites/door_Idle.png", 0);

    // //animations
    triImageAnimation *playerAnimationIdle = triImageAnimationFromSheet(playerSpriteIdle, 32.0f, 32.0f, 11, 1, 100);
    triImageAnimationStart(playerAnimationIdle);

    triImageAnimation *playerAnimationRun = triImageAnimationFromSheet(playerSpriteRun, 32.0f, 32.0f, 12, 1, 100);
    triImageAnimationStart(playerAnimationRun);

    triImageAnimation *playerAnimationHeart = triImageAnimationFromSheet(playerSpriteHeart, 18.0f, 14.0f, 8, 1, 100);
    triImageAnimationStart(playerAnimationHeart);

    triImageAnimation *enemyAnimationRun = triImageAnimationFromSheet(enemySpriteRun, 32.0f, 32.0f, 16, 1, 100);
    triImageAnimationStart(enemyAnimationRun);

    triImageAnimation *sawAnimationRun = triImageAnimationFromSheet(sawSprite, 38.0f, 38.0f, 8, 1, 100);
    triImageAnimationStart(sawAnimationRun);

    triTimer *deltaTime = triTimerCreate();

    bool fixPlatformCollision = true;

    while (isRunning())
    {
        triClear(0x301F21); //color a b g r

        triTimerUpdate(deltaTime);

        sceCtrlReadBufferPositive(&pad, 1);

        if (pad.Buttons != 0)
        {
            if (pad.Buttons & PSP_CTRL_RIGHT)
            {
                player.x += 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
                player.isRight = true;
            }
            else if (pad.Buttons & PSP_CTRL_LEFT)
            {
                player.x -= 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
                player.isRight = false;
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
        
        camera.followTarget(player.x, player.y, player.width, COL, ROW, 32);


        playerFeet.x = player.x + 10.0f;
        playerFeet.y = player.y + 10.0f;

        playerRight.x = player.x + 15.0f;
        playerRight.y = player.y;

        playerLeft.x = player.x;
        playerLeft.y = player.y;

        if (player.isJump)
        {
            player.y -= 1.0f * player.jumpHeight * triTimerPeekDeltaTime(deltaTime);
            player.jumpHeight -= 10.0f;

            fixPlatformCollision = false;
        }

        //floor collision
        if (checkPlayerFloorCollision(&playerFeet, &floor) || checkPlayerFloorCollision(&playerFeet, &floor1) || checkPlayerFloorCollision(&playerFeet, &floor2) || checkPlayerFloorCollision(&playerFeet, &floor3))
        {
            // floor detected
            player.onFloor = true;
            player.isJump = false;
            player.jumpHeight = 400.0f;

            //player.y -= 3.0f;
            //camera.getY() += 3.0f;
        }
        else
        {
            player.onFloor = false;
            player.isAttack = false;
        }

        if (!player.onFloor)
        {
            player.y += gravity * triTimerPeekDeltaTime(deltaTime);
            //camera.getY() -= gravity * triTimerPeekDeltaTime(deltaTime);
        }
        if (!fixPlatformCollision && player.onFloor)
        {
            player.y -= 5.0f;
            fixPlatformCollision = true;
        }
        //wall collision
        if (checkPlayerFloorCollision(&playerRight, &floor1) || checkPlayerFloorCollision(&playerRight, &floor2) || checkPlayerFloorCollision(&playerRight, &rightWall))
        {
            player.x -= 5.0f;
        }
        if (checkPlayerFloorCollision(&playerLeft, &floor1) || checkPlayerFloorCollision(&playerLeft, &floor2) || checkPlayerFloorCollision(&playerLeft, &leftWall))
        {
            player.x += 5.0f;
        }


        en1.feet.x = en1.x;
        en1.feet.y = en1.y + 10.f;

        en1.colliderLeft.x = en1.x - 10.f;
        en1.colliderLeft.y = en1.y;

        en1.colliderRight.x = en1.x + 10.f;
        en1.colliderRight.y = en1.y;

        if(checkCollision(&en1.feet, &floor))
        {
            en1.onFloor = true;
        }
        else
            en1.onFloor = false;

        if (!en1.onFloor)
        {
            en1.y += gravity * triTimerPeekDeltaTime(deltaTime);
        }
        if(checkCollision(&en1.colliderRight, &floor1))
        {
            en1.x -= 1.0f;
        }

        /*ai*/
        float distance = getDistance(player.x, player.y, en1.x - camera.getX(), en1.y - camera.getY());
        if(!en1.isDead)
        {
            if (distance < 150.0f)
            {
                if (player.x < en1.x - camera.getX())
                {
                    en1.x -= 1.0f * en1.speed * triTimerPeekDeltaTime(deltaTime);
                    en1.isMoving = true;
                    //en1.isAttack = false;
                }
                else if (player.x > en1.x - camera.getX())
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

        if(checkPlayerEnemyCollision(&playerFeet, &en1) && !en1.isDead)
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
        if(checkPlayerCollision(&playerRight, &en1.colliderLeft) && !en1.isDead)
        {
            player.x -= 1.0f * 1000.0f * triTimerPeekDeltaTime(deltaTime);
            player.isJump = true;
            player.onFloor = false;
            player.jumpHeight = 250.0f;
            player.health--;
        }
        if(checkPlayerCollision(&playerLeft, &en1.colliderRight) && !en1.isDead)
        {
            player.x += 1.0f * 1000.0f * triTimerPeekDeltaTime(deltaTime);
            player.isJump = true;
            player.onFloor = false;
            player.jumpHeight = 250.0f;
            player.health--;
        }
       
        for (int y = 0; y < ROW; y++)
        {
            for (int x = 0; x < COL; x++)
            {
                if(map[y][x] == 1)//bbb8d0 //aabace //bccbd5
                    triDrawRect(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 0xffcebaaa);
                else if(map[y][x] == 2)//bccbd5
                    triDrawRect(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 0xffd5cbbc);
                else if(map[y][x] == 3)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 16, 0, 32, 16, terrainSpriteSheet);
                else if(map[y][x] == 12)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 0, 0, 16, 16, terrainSpriteSheet);
                else if(map[y][x] == 15)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 32, 0, 48, 16, terrainSpriteSheet);
                else if(map[y][x] == 11)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 32, 16, 48, 32, terrainSpriteSheet);
                else if(map[y][x] == 14)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 0, 16, 16, 32, terrainSpriteSheet);
                else if(map[y][x] == 7)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 80, 0, 96, 16, terrainSpriteSheet);
                else if(map[y][x] == 8)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 96, 0, 112, 16, terrainSpriteSheet);
                else if(map[y][x] == 9)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 112, 0, 128, 16, terrainSpriteSheet);
                else if(map[y][x] == 10)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 80, 16, 96, 32, terrainSpriteSheet);
                else if(map[y][x] == 16)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 96, 16, 112, 32, terrainSpriteSheet);
                else if(map[y][x] == 17)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 112, 16, 128, 32, terrainSpriteSheet);
                else if(map[y][x] == 18)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 80, 32, 96, 48, terrainSpriteSheet);
                else if(map[y][x] == 19)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 96, 32, 112, 48, terrainSpriteSheet);
                else if(map[y][x] == 20)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 112, 32, 128, 48, terrainSpriteSheet);
                
                //wall
                else if(map[y][x] == 21)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 0, 32, 16, 48, terrainSpriteSheet);
                else if(map[y][x] == 13)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 16, 32, 32, 48, terrainSpriteSheet);
                
                //block
                else if(map[y][x] == 22)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 176, 32, 192, 48, terrainSpriteSheet);


                //draw decor
                /*if (decor[y][x] == 1)
                    triDrawImage(x * 32 - camera.getX(), y * 32 - camera.getY(), 32, 32, 64, 96, 96, 128, decorSpriteSheet);
                */
            }
        }
        
        //draw floor
        //triDrawRect(floor.x - camera.getX(), floor.y - camera.getY(), floor.width, floor.height, 0xff00ffff);
        //triDrawRect(floor1.x - camera.getX(), floor1.y - camera.getY(), floor1.width, floor1.height, 0xff00ffff);
        //triDrawRect(floor2.x - camera.getX(), floor2.y - camera.getY(), floor2.width, floor2.height, 0xff00ffff);
        //triDrawRect(floor3.x - camera.getX(), floor3.y - camera.getY(), floor3.width, floor3.height, 0xff00ffff);


        //triDrawRect(leftWall.x - camera.getX(), leftWall.y - camera.getY(), leftWall.width, leftWall.height, 0xff00ffff);
        //triDrawRect(rightWall.x - camera.getX(), rightWall.y - camera.getY(), rightWall.width, rightWall.height, 0xff00ffff);

        //draw door
        //triDrawSprite(80.0f - camera.getX(), 272.0f / 2.0f + 32.0f - camera.getY(), 0, 0, doorSprite);

        //draw enemy
        //triDrawRect(en1.x - camera.getX(), en1.y - camera.getY(), en1.width, en1.height, 0xff0000ff);


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
            triDrawSprite(en1.x - 32.0f / 2.0f + en1.width - 10.0f - camera.getX(), en1.y - 32.0f / 2.0f - camera.getY(), 0, 0, enemySpriteIdle);
        }
        else if (en1.isMoving && !en1.isDead)
        {
            triDrawImageAnimation(en1.x - 32.0f / 2.0f + en1.width - 10.0f - camera.getX(), en1.y - 32.0f / 2.0f - camera.getY(), enemyAnimationRun);
            triImageAnimationUpdate(enemyAnimationRun);
        }

        //triDrawImage(floor.x, floor.y, 32, 32, 64, 32, 96, 64, terrainSpriteSheet);

        //draw player
        //triDrawRect(player.x, player.y, player.width, player.height, 0xff0000ff);
        
        
        if (!player.isMoving && player.onFloor)
        {
            triDrawImageAnimation(player.x - 32.0f / 2.0f + player.width - camera.getX(), player.y - 32.0f / 2.0f - camera.getY(), playerAnimationIdle);
            triImageAnimationUpdate(playerAnimationIdle);
        }
        else if (player.isMoving && player.onFloor)
        {
            triDrawImageAnimation(player.x - 32.0f / 2.0f + player.width - camera.getX(), player.y - 32.0f / 2.0f - camera.getY(), playerAnimationRun);
            triImageAnimationUpdate(playerAnimationRun);
        }
        else
        {
            triDrawSprite(player.x - 32.0f / 2.0f + player.width - camera.getX(), player.y - 32.0f / 2.0f - camera.getY(), 0, 0, playerSpriteJump);
        }

            //triDrawImageAnimation(480/2, 272/2, sawAnimationRun);
            //triImageAnimationUpdate(sawAnimationRun);

/*
        triDrawRect(playerLeft.x, playerLeft.y, playerLeft.width, playerLeft.height, 0xff0000ff);
        triDrawRect(playerRight.x, playerRight.y, playerRight.width, playerRight.height, 0xff0000ff);
        triDrawRect(playerFeet.x, playerFeet.y, playerFeet.width, playerFeet.height, 0xff0000ff);

        triDrawRect(enLeft1.x - camera.getX(), enLeft1.y - camera.getY(), enLeft1.width, enLeft1.height, 0xff0000ff);
        triDrawRect(enRight1.x - camera.getX(), enRight1.y - camera.getY(), enRight1.width, enRight1.height, 0xff0000ff);
        triDrawRect(enFeet1.x - camera.getX(), enFeet1.y - camera.getY(), enFeet1.width, enFeet1.height, 0xff0000ff);
*/

        triFontActivate(0);
        triFontPrintf(0, 0, 0xFFFFFFFF, "FPS: %.2f - MAX: %.2f - MIN: %.2f", triFps(), triFpsMax(), triFpsMin());
        triFontPrintf(0, 10, 0xFFFFFFFF, "CPU: %.2f%% - GPU: %.2f%%", triCPULoad(), triGPULoad());
        triFontPrintf(0, 20, 0xFFFFFFFF, "Player: x = %.2f , y = %.2f", player.x - camera.getX(), player.y - camera.getY());
        triFontPrintf(0, 30, 0xFFFFFFFF, "onFloor: %d", player.onFloor);
        triFontPrintf(0, 40, 0xFFFFFFFF, "Camera: x = %.2f , y = %.2f", camera.getX(), camera.getY());
        triFontPrintf(0, 50, 0xFFFFFFFF, "Distance: %.2f", getDistance(player.x, player.y, en1.x - camera.getX(), en1.y - camera.getY()));
        triFontPrintf(0, 60, 0xFFFFFFFF, "Health: %d", player.health);


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
    triImageFree(sawSprite);
    triTimerFree(deltaTime);
    triClose();
    triMemoryCheck();
    triMemoryShutdown();

    sceKernelExitGame();

    return 0;
}