#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspdebug.h>
#include <pspctrl.h>

#include <math.h>
#include <vector>

#include "callback.h"

extern "C" {
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
float vfpu_sqrtf(float x) {
	float result;
	__asm__ volatile (
		"mtv     %1, S000\n"
		"vsqrt.s S000, S000\n"
		"mfv     %0, S000\n"
	: "=r"(result) : "r"(x));
	return result;
}

typedef struct Collider {
    float x, y;
    float width, height;
} Collider;

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

typedef struct Enemy {
    float x, y;
    float width, height;
    float speed;
    bool onFloor;
    bool isMoving;
    bool isRight;
} Enemy;

typedef struct Floor {
    float x, y;
    float width, height;
} Floor;

typedef struct Camera {
    float x, y;
} Camera;

const int ROW = 10;
const int COL = 25;

int map[ROW][COL] = { 
                    {11,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13,13, 0} , 
                    {11, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 5,14} , 
                    {11, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6,14} ,
                    {11, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6,14} ,
                    {11, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 6,14} ,
                    {11, 9, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7, 7, 7, 7, 7, 1, 1, 1, 6,14} ,                   
                    {11,10, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 3, 3, 3, 3, 3,10, 1, 1, 6,14} ,
                    {12, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 3,10, 7, 8,14} , //floor
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3} ,  
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}          
                };

int decor[ROW][COL] = { 
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , 
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , 
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 0, 0, 1, 2, 0, 5, 6, 0, 1, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 0, 0, 3, 4, 0, 7, 8, 0, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,                   
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} , //floor
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0} ,  
                    { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}          
                };

bool checkCollision(Collider *p, Floor *f, Camera *c)
{
    if(p->x < f->x + f->width + c->x &&
            p->x + p->width > f->x + c->x &&
            p->y < f->y + f->height + c->y &&
            p->y + p->height > f->y + c->y) 
        return true;
    else 
        return false;
}

float getDistance(float x1, float y1, float x2, float y2)
{
    float a = (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1);
    a = vfpu_sqrtf(a);
    if(isnan(a) == 0)
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

    Player player { 480.0f/2.0f, 272.0f/2.0f, 10.0f, 10.0f, 50.0f, 400.0f, false, false, false, true};
    Collider playerFeet { player.x, player.y, 5.0f, 5.0f};
    Collider playerRight { player.x, player.y, 5.0f, 5.0f};
    Collider playerLeft { player.x, player.y, 5.0f, 5.0f};

    Enemy en1 { 480.0f/2.0f + 64.0f, 272.0f/2.0f, 10.0f, 10.0f, 10.0f, false, false, false};

    Camera camera { 0, 0};

    float gravity = 100.0f;

    Floor floor { 0, 224.0f, 480.0f, 32.0f};
    Floor floor1 { 480.0f, 192.0f, 160.0f, 32.0f};
    Floor floor2 { 640.0f, 224.0f, 32.0f, 32.0f};
    Floor floor3 { 672.0f, 256.0f, 128.0f, 32.0f};

    Floor leftWall { 0, 0, 32.0f, 256.0f};
    Floor rightWall { 768.0f, 0, 32.0f, 288.0f};

    //load sprites
    triImage* playerSpriteIdle = triImageLoad("assets/sprites/king_Idle (78x58).png", 0);
    triImage* playerSpriteRun = triImageLoad("assets/sprites/king_Run (78x58).png" , 0);
    triImage* playerSpriteJump = triImageLoad("assets/sprites/king_Jump (78x58).png" , 0);

    triImage* terrainSpriteSheet = triImageLoad("assets/sprites/Terrain (32x32).png" , 0);
    triImage* decorSpriteSheet = triImageLoad("assets/sprites/Decorations (32x32).png" , 0);

    triImage* enemySpriteIdle = triImageLoad("assets/sprites/pig_Idle (34x28).png" , 0);
    triImage* enemySpriteRun = triImageLoad("assets/sprites/pig_Run (34x28).png" , 0);

    triImage* doorSprite = triImageLoad("assets/sprites/door_Idle.png", 0);
    
    //animations
    triImageAnimation* playerAnimationIdle = triImageAnimationFromSheet(playerSpriteIdle, 78.0f, 58.0f, 11, 1, 100);
    triImageAnimationStart(playerAnimationIdle);

    triImageAnimation* playerAnimationRun = triImageAnimationFromSheet(playerSpriteRun,  78.0f, 58.0f, 8, 1, 100);
    triImageAnimationStart(playerAnimationRun);

    triImageAnimation* enemyAnimationIdle = triImageAnimationFromSheet(enemySpriteIdle, 34.0f, 28.0f, 11, 1, 100);
    triImageAnimationStart(enemyAnimationIdle);

    triImageAnimation* enemyAnimationRun = triImageAnimationFromSheet(enemySpriteRun,  34.0f, 28.0f, 6, 1, 100);
    triImageAnimationStart(enemyAnimationRun);

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
                //player.x += 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
                player.isRight = true;

                camera.x -= 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
            }
            else if(pad.Buttons & PSP_CTRL_LEFT)
            {
                //player.x -= 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
                player.isMoving = true;
                player.isRight = false;

                camera.x += 1.0f * player.speed * triTimerPeekDeltaTime(deltaTime);
            }
            if(pad.Buttons & PSP_CTRL_UP && player.onFloor)
            {
                player.isJump = true;
                player.onFloor = false;
            }
        }
        if(pad.Buttons == 0)
            player.isMoving = false;

        playerFeet.x = player.x;
        playerFeet.y = player.y + 10.0f;

        playerRight.x = player.x + 7.0f;
        playerRight.y = player.y;

        playerLeft.x = player.x - 7.0f;
        playerLeft.y = player.y;

        if(player.isJump)
        {
            //player.y -= 1.0f * player.jumpHeight * triTimerPeekDeltaTime(deltaTime);
            player.jumpHeight -= 10.0f;

            camera.y += 1.0f * player.jumpHeight * triTimerPeekDeltaTime(deltaTime);
        }

        //floor collision
        if(checkCollision(&playerFeet, &floor, &camera) || checkCollision(&playerFeet, &floor1, &camera) || checkCollision(&playerFeet, &floor2, &camera) || checkCollision(&playerFeet, &floor3, &camera))
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
        }


        if(!player.onFloor)
        {
            //player.y += gravity * triTimerPeekDeltaTime(deltaTime);
            camera.y -= gravity * triTimerPeekDeltaTime(deltaTime);
        }
        //wall collision
        if(checkCollision(&playerRight, &floor1, &camera) || checkCollision(&playerRight, &floor2, &camera) || checkCollision(&playerRight, &rightWall, &camera))
        {
            camera.x += 1.0f;
        }
        if(checkCollision(&playerLeft, &floor1, &camera) || checkCollision(&playerLeft, &floor2, &camera) || checkCollision(&playerLeft, &leftWall, &camera))
        {
            camera.x -= 1.0f;
        }

        
        if(en1.x + camera.x < floor.x + floor.width + camera.x &&
            en1.x + camera.x + en1.width > floor.x + camera.x &&
            en1.y + camera.y < floor.y + floor.height + camera.y &&
            en1.y + en1.height + camera.y > floor.y + camera.y) 
        {
            en1.onFloor = true;
        }
        else 
            en1.onFloor = false;

        if(!en1.onFloor)
        {
           en1.y += gravity * triTimerPeekDeltaTime(deltaTime); 
        }

        /*ai*/
        if(getDistance(player.x, player.y, en1.x + camera.x, en1.y + camera.y) < 150.0f)
        {
            if(player.x < en1.x + camera.x) 
            {
                en1.x -= 1.0f * en1.speed * triTimerPeekDeltaTime(deltaTime); 
                en1.isMoving = true;
            }
            else if(player.x > en1.x + camera.x)
            {
                en1.x += 1.0f * en1.speed * triTimerPeekDeltaTime(deltaTime);   
                en1.isMoving = true;  
            }
            else 
            {
                en1.x = en1.x;
            }
        }
        else 
        {
            en1.isMoving = false;
        }

        



        for(int y = 0;y < ROW;y++)
        {
            for(int x = 0;x < COL;x++)
            {
                if(map[y][x] == 1)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 64, 256, 96, 288, terrainSpriteSheet);
                else if(map[y][x] == 2)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 64, 224, 96, 256, terrainSpriteSheet); //draw wall background
                else if(map[y][x] == 3)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 64, 32, 96, 64, terrainSpriteSheet); //draw floor
                else if(map[y][x] == 4)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 32, 224, 64, 256, terrainSpriteSheet);
                else if(map[y][x] == 5)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 96, 224, 128, 256, terrainSpriteSheet);
                else if(map[y][x] == 6)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 96, 256, 128, 288, terrainSpriteSheet);
                else if(map[y][x] == 7)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 64, 288, 96, 320, terrainSpriteSheet);
                else if(map[y][x] == 8)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 96, 288, 128, 320, terrainSpriteSheet);
                else if(map[y][x] == 9)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 32, 256, 64, 288, terrainSpriteSheet);
                else if(map[y][x] == 10)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 32, 288, 64, 320, terrainSpriteSheet);
                else if(map[y][x] == 11)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 96, 64, 128, 96, terrainSpriteSheet);
                else if(map[y][x] == 12)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 352, 32, 383, 64, terrainSpriteSheet);
                else if(map[y][x] == 13)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 64, 96, 96, 128, terrainSpriteSheet);
                else if(map[y][x] == 14)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 32, 64, 64, 96, terrainSpriteSheet);                


                /*draw decor*/
                if(decor[y][x] == 1)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 64, 96, 96, 128, decorSpriteSheet);
                else if(decor[y][x] == 2)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 96, 96, 128, 128, decorSpriteSheet);
                else if(decor[y][x] == 3)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 64, 128, 96, 160, decorSpriteSheet);
                else if(decor[y][x] == 4)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 96, 128, 128, 160, decorSpriteSheet);
                
                else if(decor[y][x] == 5)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 128, 96, 160, 128, decorSpriteSheet);
                else if(decor[y][x] == 6)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 160, 96, 192, 128, decorSpriteSheet);
                else if(decor[y][x] == 7)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 128, 128, 160, 160, decorSpriteSheet);
                else if(decor[y][x] == 8)
                    triDrawImage(x * 32 + camera.x, y * 32 + camera.y, 32, 32, 160, 128, 192, 160, decorSpriteSheet); 
                
            }
        }

        //draw floor
        //triDrawRect(rightWall.x + camera.x, rightWall.y + camera.y, rightWall.width, rightWall.height, 0xff00ffff);
        
        //draw door
        triDrawSprite(80.0f + camera.x, 272.0f/2.0f + 32.0f + camera.y, 0, 0, doorSprite);

        //draw enemy
        triDrawRect(en1.x + camera.x, en1.y + camera.y, en1.width, en1.height, 0xff0000ff);

        if(!en1.isMoving)
        {
            triDrawImageAnimation(en1.x - 34.0f/2.0f + en1.width - 10.0f + camera.x, en1.y - 28.0f/2.0f + camera.y, enemyAnimationIdle);
            triImageAnimationUpdate(enemyAnimationIdle);
        }
        else if(en1.isMoving)
        {
            triDrawImageAnimation(en1.x - 34.0f/2.0f + en1.width - 10.0f + camera.x, en1.y - 28.0f/2.0f + camera.y, enemyAnimationRun);
            triImageAnimationUpdate(enemyAnimationRun);       
        }

        // triDrawImage(floor.x, floor.y, 32, 32, 64, 32, 96, 64, terrainSpriteSheet);


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

        // if(player.onFloor)
        //     triDrawRect(playerLeft.x, playerLeft.y, playerLeft.width, playerLeft.height, 0xff00ff00);
        // else
        //     triDrawRect(playerLeft.x, playerLeft.y, playerLeft.width, playerLeft.height, 0xff0000ff);

        triFontActivate(0);
        triFontPrintf(0 , 0, 0xFFFFFFFF, "FPS: %.2f - MAX: %.2f - MIN: %.2f", triFps(), triFpsMax(), triFpsMin());
        triFontPrintf(0 , 10, 0xFFFFFFFF, "CPU: %.2f%% - GPU: %.2f%%", triCPULoad(), triGPULoad());
        triFontPrintf(0, 20, 0xFFFFFFFF, "Player: x = %.2f , y = %.2f", player.x  - camera.x, player.y - camera.y);
        triFontPrintf(0, 30, 0xFFFFFFFF, "onFloor: %d", player.onFloor);
        triFontPrintf(0, 40, 0xFFFFFFFF, "Camera: x = %.2f , y = %.2f", camera.x, camera.y);
        triFontPrintf(0, 50, 0xFFFFFFFF, "Distance: %.2f", getDistance(player.x, player.y, en1.x + camera.x, en1.y + camera.y));

        triSwapbuffers();
    }

    triImageFree(enemySpriteIdle);
    triImageFree(playerSpriteIdle);
    triImageFree(playerSpriteRun);
    triImageFree(playerSpriteJump);
    triImageFree(decorSpriteSheet);
    triImageFree(terrainSpriteSheet);
    triImageFree(doorSprite);
    triTimerFree(deltaTime);
    triClose();
    triMemoryCheck();
    triMemoryShutdown();

    sceKernelExitGame();

    return 0;
}