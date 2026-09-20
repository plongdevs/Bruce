#include "tururururu.h"
#include "core/display.h"
#include "core/mykeyboard.h"
#include <globals.h>

// Modded for Bruce Firmware CYD

int sharkX = 40;
int sharkY = 80;
int prevSharkX = 40; // Lưu vị trí cũ để xóa vết
int prevSharkY = 80;
int sharkSize = 14;
bool sharkUp = false;
bool sharkDown = false;

struct Fish {
    int x, y, size;
    int prevX, prevY; // Lưu vị trí cũ để xóa vết
    bool isBomb;
};
Fish fish[5];

struct HeartItem {
    int x, y;
    int prevX, prevY;
    bool active;
};
HeartItem healHeart;

int score = 0;
int lives = 3;

void initSprites() {
    tft.fillScreen(bruceConfig.bgColor);

    // Sprite Cá mập (32x30)
    sprite.deleteSprite();
    sprite.createSprite(32, 30);
    sprite.fillScreen(bruceConfig.bgColor);
    sprite.fillEllipse(19, 17, 10, 5, TFT_DARKGREY);
    sprite.fillCircle(17, 24, 5, TFT_LIGHTGREY);
    sprite.fillTriangle(0, 10, 0, 22, 9, 17, TFT_DARKGREY);
    sprite.fillTriangle(17, 6, 17, 14, 22, 14, TFT_DARKGREY);
    sprite.fillCircle(25, 14, 1, TFT_RED);
    sprite.fillTriangle(23, 18, 29, 18, 24, 21, TFT_RED);
    sprite.fillRect(0, 21, 32, 15, bruceConfig.bgColor);

    // Sprite Cá mồi (20x8)
    draw.deleteSprite();
    draw.createSprite(20, 8);

    healHeart.active = false;
}

void drawShark() {
    // Xóa vị trí cũ của cá mập bằng màu nền trước khi vẽ vị trí mới
    if (prevSharkY != sharkY || prevSharkX != sharkX) {
        tft.fillRect(prevSharkX - sharkSize, prevSharkY - 7, 32, 30, bruceConfig.bgColor);
    }
    sprite.pushSprite(sharkX - sharkSize, sharkY - 7);
    
    // Cập nhật lại vị trí cũ
    prevSharkX = sharkX;
    prevSharkY = sharkY;
}

void drawFish(Fish &f) {
    // Xóa vết cũ của con cá này trước khi đẩy sprite mới
    tft.fillRect(f.prevX, f.prevY, 20, 8, bruceConfig.bgColor);

    uint16_t fishColor = f.isBomb ? TFT_RED : TFT_ORANGE;
    draw.fillScreen(bruceConfig.bgColor);
    draw.fillEllipse(6, 4, 6, 3, fishColor);
    draw.fillTriangle(16, 0, 16, 8, 11, 5, fishColor);
    draw.drawFastVLine(6, 1, 7, TFT_WHITE);
    draw.drawFastVLine(10, 1, 7, TFT_WHITE);
    draw.drawFastVLine(15, 1, 7, TFT_WHITE);
    draw.fillCircle(3, 3, 1, TFT_BLACK);
    draw.pushSprite(f.x, f.y);

    f.prevX = f.x;
    f.prevY = f.y;
}

void drawHeartItem() {
    if (healHeart.active) {
        // Xóa vết cũ trái tim
        tft.fillRect(healHeart.prevX - 3, healHeart.prevY - 3, 11, 11, bruceConfig.bgColor);

        tft.fillCircle(healHeart.x, healHeart.y, 3, TFT_RED);
        tft.fillCircle(healHeart.x + 4, healHeart.y, 3, TFT_RED);
        tft.fillTriangle(healHeart.x - 3, healHeart.y + 1, healHeart.x + 7, healHeart.y + 1, healHeart.x + 2, healHeart.y + 7, TFT_RED);
        
        healHeart.prevX = healHeart.x;
        healHeart.prevY = healHeart.y;
    }
}

#define STEP (tftHeight) / 36 

void detectInputs() {
#if defined(ARDUINO_M5STICK_C_PLUS) || defined(ARDUINO_M5STICK_C_PLUS2)
    if (check(SelPress))
#else
    if (check(PrevPress) || check(UpPress))
#endif
    {
        sharkUp = true;
    }
    if (check(NextPress) || check(DownPress)) sharkDown = true;
}

void moveShark() {
    if (sharkDown) {
        sharkY += STEP;
        sharkDown = false;
    }
    if (sharkUp) {
        sharkY -= STEP;
        sharkUp = false;
    }
    if (sharkY < 0) { sharkY = 0; }
    if (sharkY > tftHeight - sharkSize) { sharkY = tftHeight - sharkSize; }
}

void moveFish(Fish &f) {
    f.x -= 4; 
    if (f.x < -20) {
        tft.fillRect(f.prevX, f.prevY, 20, 8, bruceConfig.bgColor);
        f.x = tftWidth + random(20, 100);
        f.y = random(15, tftHeight - 20);
        f.prevX = f.x;
        f.prevY = f.y;
        f.isBomb = (random(0, 100) < 30); 
    }
}

void moveHeartItem() {
    if (!healHeart.active && random(0, 300) < 2) {
        healHeart.active = true;
        healHeart.x = tftWidth + 10;
        healHeart.y = random(20, tftHeight - 20);
        healHeart.prevX = healHeart.x;
        healHeart.prevY = healHeart.y;
    }

    if (healHeart.active) {
        healHeart.x -= 3;
        if (healHeart.x < -10) {
            tft.fillRect(healHeart.prevX - 3, healHeart.prevY - 3, 11, 11, bruceConfig.bgColor);
            healHeart.active = false;
        }
    }
}

void checkCollisions() {
    for (int i = 0; i < 5; i++) {
        if ((sharkX < fish[i].x + fish[i].size) && (sharkX + sharkSize > fish[i].x) &&
            (sharkY < fish[i].y + fish[i].size) && (sharkY + sharkSize > fish[i].y)) {
            
            tft.fillRect(fish[i].x - 5, fish[i].y - 5, 30, 20, bruceConfig.bgColor);
            
            if (fish[i].isBomb) {
                lives--; 
                tft.fillScreen(TFT_RED);
                delay(100);
                tft.fillScreen(bruceConfig.bgColor);
            } else {
                score++; 
            }

            fish[i].x = tftWidth + random(20, 100);
            fish[i].y = random(15, tftHeight - 20);
            fish[i].prevX = fish[i].x;
            fish[i].prevY = fish[i].y;
            fish[i].isBomb = (random(0, 100) < 30);
        }
    }

    if (healHeart.active) {
        if ((sharkX < healHeart.x + 8) && (sharkX + sharkSize > healHeart.x) &&
            (sharkY < healHeart.y + 8) && (sharkY + sharkSize > healHeart.y)) {
            
            tft.fillRect(healHeart.x - 5, healHeart.y - 5, 20, 20, bruceConfig.bgColor);
            healHeart.active = false;
            
            if (lives < 3) {
                lives++; 
            }
        }
    }
}

void displayHUD() {
    tft.setTextColor(TFT_WHITE, bruceConfig.bgColor);
    tft.setTextSize(FM);
    tft.setCursor(2, 2);
    tft.printf("Score: %d ", score);

    int heartStartX = tftWidth - 45;
    for (int i = 0; i < 3; i++) {
        uint16_t color = (i < lives) ? TFT_RED : TFT_DARKGREY;
        int hX = heartStartX + (i * 14);
        int hY = 5;
        tft.fillCircle(hX, hY, 2, color);
        tft.fillCircle(hX + 3, hY, 2, color);
        tft.fillTriangle(hX - 2, hY + 1, hX + 5, hY + 1, hX + 1, hY + 5, color);
    }
}

void shark_setup() {
    score = 0;
    lives = 3; 

    for (int i = 0; i < 5; i++) {
        fish[i].x = tftWidth + random(20, 100);
        fish[i].y = random(15, tftHeight - 20);
        fish[i].prevX = fish[i].x;
        fish[i].prevY = fish[i].y;
        fish[i].size = 8;
        fish[i].isBomb = (i == 4); 
    }
    
    initSprites();
    if (tft.getLogging()) tft.log_drawString("Not Supported", DRAWCENTRESTRING, tftWidth / 2, tftHeight / 2);
    
    shark_loop();
}

void shark_loop() {
    int downTime = 30; 
    unsigned long time = 0;

    for (;;) {
        displayHUD();
        detectInputs();

        if (millis() - time > downTime) {
            moveShark();
            drawShark();

            for (int i = 0; i < 5; i++) {
                moveFish(fish[i]);
                drawFish(fish[i]);
            }

            moveHeartItem();
            drawHeartItem();

            checkCollisions();
            time = millis();
        }

        if (lives <= 0) {
            tft.fillScreen(TFT_RED);
            tft.setTextColor(TFT_WHITE, TFT_RED);
            tft.drawCentreString("GAME OVER!", tftWidth / 2, tftHeight / 2 - 10, 2);
            delay(1500);
            break; 
        }

        if (check(EscPress)) {
            returnToMenu = true;
            goto Exit;
        }
    }

Exit:
    delay(150);
    Serial.println();
}