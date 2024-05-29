#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <gccore.h>
#include <ogc/machine/processor.h>
#include <libgui.h>

#include "discAnim.h"

// Flag to insert disc
u8 insertDisc = 0;

int discAnim(u8* arcData, std::string gameIDString) {
    float yOffset = 300;
    float xOffset = 800;
    float gap = 44;
    float discRotation = 225;
    float rotAcc = 1;
    float rotDec = 4.5;
    float yMovement = 0;
    float yAcc = 0;
    float yDec = 1;
    float yTravel = 22;
    float yFall = 40;
    float trayMidWidth = 280;
    float trayEdgeWidth = 9;
    float trayHeight = 6;
    f32 shadeTexCoords[8] = {0.0f, 2.0f, 2.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f};
    f32 trayLeftTexCoords[8] = {0.0f, 2.0f, 1.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f};
    f32 trayCenterTexCoords[8] = {0.9f, 2.0f, 1.1f, 2.0f, 1.1f, 0.0f, 0.9f, 0.0f};
    f32 trayRightTexCoords[8] = {1.0f, 2.0f, 2.0f, 2.0f, 2.0f, 0.0f, 1.0f, 0.0f};
    f32 trayScale = 0.1f;
    u16 shadeAlpha = 0;
    u16 discAlpha = 0;
    u8 fall = 1;
    u16 shadeSize = 576;
    GuiImage discImage;
    TPLFile wiiDiscTPL, shadeTPL;
    char tempPath[PATH_MAX];
    
    if (CONF_GetAspectRatio() == CONF_ASPECT_4_3) xOffset = 640;
    
    sprintf(tempPath, "%s/%s.png", DISC_PATH, gameIDString.c_str());
    FILE* fp = fopen(tempPath, "rb");
    if(fp){
        fclose(fp);
        discImage = GuiImage(tempPath);
        discImage.setSize(256, 256);
    }
    else{
        u32 wiiDiscSize = 131136;
        TPL_OpenTPLFromMemory(&wiiDiscTPL, arcData+0x4E7A0, wiiDiscSize);
        discImage = GuiImage(&wiiDiscTPL,0);
    }
    Vector2 discImageDimensions = discImage.getDimensions();
    
    TPL_OpenTPLFromMemory(&shadeTPL, arcData+0x3C4E0, shadeSize);
    GuiImage shadeImage(&shadeTPL,0);
    shadeImage.setTextureWrap(GX_MIRROR, GX_MIRROR);
    Vector2 shadeImageDimensions = shadeImage.getDimensions();

    while(1) {
        Gfx::startDrawing();
        Gfx::pushMatrix();
        Gfx::translate((xOffset - discImageDimensions.x) / 2, yOffset - discImageDimensions.y - gap / 2 - yFall);
        Gfx::pushScissorBox(discImageDimensions.x + 1, discImageDimensions.y + yTravel + yFall);
        Gfx::translate(0, yMovement);
        Gfx::rotate(discImageDimensions.x / 2, discImageDimensions.y / 2, discRotation);
        discImage.setSize(discImageDimensions.x, discImageDimensions.y);
        discImage.drawAlpha(discAlpha);
        Gfx::popScissorBox();
        Gfx::popMatrix();

        Gfx::pushMatrix();
        Gfx::translate((xOffset - discImageDimensions.x) / 2, yOffset + gap / 2 - yTravel);
        Gfx::pushScissorBox(discImageDimensions.x, discImageDimensions.y/2 + yTravel);
        Gfx::pushMatrix();
        Gfx::translate(0, + yTravel - yMovement + yFall);
        Gfx::rotate(discImageDimensions.x / 2, discImageDimensions.y / 2, -discRotation);
        discImage.setSize(discImageDimensions.x, -discImageDimensions.y);
        discImage.drawAlpha(discAlpha);
        Gfx::popMatrix();
        
        //Draw disc mask
        draw4ColorsRectangle(-1, 0, discImageDimensions.x + 1, discImageDimensions.y/1.8 + yTravel, RGBA8(45, 45, 45, 150), RGBA8(45, 45, 45, 150), RGBA8(45, 45, 45, 255), RGBA8(45, 45, 45, 255));
        Gfx::popScissorBox();
        Gfx::popMatrix();
        
        //Draw shade
        Gfx::pushMatrix();
        Gfx::translate((xOffset-discImageDimensions.x) / 2, yOffset - 5);
        shadeImage.setSize(shadeImageDimensions.x * 8, shadeImageDimensions.y * 0.36);
        shadeImage.drawTextureAlphaTexCoords(shadeAlpha,shadeTexCoords);
        Gfx::popMatrix();
        
        // Draw tray in three parts: left edge, middle, right edge
        Gfx::pushMatrix();
        Gfx::translate((xOffset - trayMidWidth * trayScale) / 2 - trayEdgeWidth * trayScale + 1, yOffset - 2);
        shadeImage.setSize(trayEdgeWidth * trayScale, trayHeight * trayScale);
        shadeImage.drawTextureAlphaTexCoords(255,trayLeftTexCoords);
        Gfx::popMatrix();
        Gfx::pushMatrix();
        Gfx::translate((xOffset - trayMidWidth * trayScale) / 2, yOffset - 2);
        shadeImage.setSize(trayMidWidth * trayScale, trayHeight * trayScale);
        shadeImage.drawTextureAlphaTexCoords(255,trayCenterTexCoords);
        Gfx::popMatrix();
        Gfx::pushMatrix();
        Gfx::translate((xOffset + trayMidWidth * trayScale) / 2 - 1, yOffset - 2);
        shadeImage.setSize(trayEdgeWidth * trayScale, trayHeight * trayScale);
        shadeImage.drawTextureAlphaTexCoords(255,trayRightTexCoords);
        Gfx::popMatrix();

        Gfx::endDrawing();

        if(yMovement < yFall && fall){
            yMovement += yDec;
            if(yAcc > 0.005) yDec /= 1.2;
            shadeAlpha += 7;
            discAlpha += 7;
            if(shadeAlpha > 255){
                shadeAlpha = 255;
                discAlpha = 255;
            }
        }
        if(discRotation > 0 && fall){
            discRotation -= rotDec;
            if(rotDec > 0.9) rotDec /= 1.02;
        }
        else fall=0;
        if(!fall){
            if(rotAcc<24) rotAcc += 0.5;
            discRotation += rotAcc;
            if(insertDisc){
                if(shadeAlpha > 0) shadeAlpha -= 51;
                if(yAcc < 8) yAcc += 1;
                if(trayScale < 1.0f && yMovement < discImageDimensions.y + yFall) trayScale += 0.1f;
                if(trayScale > 0.0f && yMovement > discImageDimensions.y + yTravel + yFall) trayScale -= 0.1f;
            }
            yMovement += yAcc;
        }
    }
    free(arcData);
    return 0;
}
