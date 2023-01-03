#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <tice.h>
#include <keypadc.h>
#include <graphx.h>

#include <draw.h>
#include <players.h>

kb_key_t key;

void clearPlayers() {
  gfx_FillScreen(255);
}

void doDeath() {
  player_t player = players[0];
  gfx_Begin();
  do {
    gfx_SetDrawBuffer();
    kb_Scan();
    gfx_FillScreen(224);
    gfx_SetTextScale(2,2);
    gfx_PrintStringXY("YOU DIED", 160 - gfx_GetStringWidth("YOU DIED") / 2,95);
    gfx_SetTextScale(1,1);
    gfx_SetColor(255);
    gfx_PrintStringXY("Press any key to continue.",160 - gfx_GetStringWidth("Press any key to continue.") / 2,117);
    gfx_PrintStringXY("Score: ", 160 - gfx_GetStringWidth("Score:   ") / 2,130);
    gfx_PrintInt(player.i,1);
    gfx_SwapDraw();
  } while (!os_GetCSC());
  dead = 0;
  srand(rtc_Time());
  createFood();
  clearPlayers();
  createPlayers();
  topI = 0;
}

void handleKeys(kb_key_t keyLocal) {
  uint8_t cloneIdx;
  
  player_t* player = &(players[0]);
  
  //if cloneCount idx is unfilled, createClones
  for (cloneIdx = 0; cloneIdx < cloneCount; cloneIdx++) {
    clone_t clone = clones[cloneIdx];
    if (kb_Data[6] & kb_Enter && clone.filled == 0) createClones(cloneIdx);
  }
  
  if (keyLocal == kb_Right) {
    player->dir++;
    if (player->dir == 9) player->dir = 1;
  } else if (keyLocal == kb_Left) {
    player->dir--;
    if (player->dir == 0) player->dir = 8;
  } else if (keyLocal == kb_Up) {
    player->dir = 1;
  } else if (keyLocal == kb_Down) {
    player->dir = 5;
  }
}

void drawItems() {
  int idx;
  int scaledRadius;
  for (idx = 0; idx < foodCount; idx++) {
    food_t food = arrFood[idx];
    gfx_SetColor(food.color);
    gfx_FillRectangle(food.x,food.y,4,4);
  }

  for (idx = 0; idx < maxPlayers; idx++) {
    player_t player = players[idx];
    //scaledRadius = player.i * resizeVar;
    gfx_SetColor(player.color);
    gfx_FillCircle(player.x, player.y, player.i);
  }
  
  for (idx = 0; idx < cloneCount; idx++) {
    clone_t clone = clones[idx];
    //scaledRadius = clone.i * resizeVar;
    gfx_SetColor(clone.parentColor);
    gfx_FillCircle(clone.x, clone.y, clone.i);
  }
  gfx_SetDrawBuffer();
}

void drawNames() {
  player_t player = players[0];
  if (dead == 0) {
    //find center
    gfx_PrintStringXY("YOU",player.x - gfx_GetStringWidth("YOU") / 2,player.y - 2);
  }
}

void dispStats() {
  player_t player = players[0];
  gfx_SetTextScale(1,1);
  //gfx_PrintStringXY("Direction: ",10,10);
  /*if (player.dir == 1) gfx_PrintString("UP");
  if (player.dir == 3) gfx_PrintString("RIGHT");
  if (player.dir == 5) gfx_PrintString("DOWN");
  if (player.dir == 7) gfx_PrintString("LEFT");*/
  if (player.dir == 1) gfx_PrintStringXY("^",player.x - 4,player.y - player.i - 8);
  if (player.dir == 3) gfx_PrintStringXY(">",player.x + player.i + 4,player.y - 4);
  if (player.dir == 5) gfx_PrintStringXY("v",player.x - 4,player.y + player.i + 4);
  if (player.dir == 7) gfx_PrintStringXY("<",player.x - player.i - 8,player.y - 4);
  
  gfx_PrintStringXY("Largest Player: ",10,10);
  gfx_PrintInt(topI,1);
  gfx_PrintStringXY("Your Size: ",10,20);
  gfx_PrintInt(player.i,1);
}

void startGame() {
  srand(rtc_Time());
  createFood();
  clearPlayers();
  createPlayers();
  findEnemy();
}

int main(void) {
  gfx_Begin();
  startGame();
  do {
    kb_Scan();
    key = kb_Data[7];
    handleKeys(key);
    moveFood();
    moveClones();
    clearPlayers();
    handleFood();
    checkPlayers();
    movePlayers();
    // draw lines
    /*drawGrid();
    for (uint8_t i = 0; i < 16) {
      gfx_HorizLine(-10,-10 + 10 * i,340);

    }*/
    //DRAW ARROWS IN THE DIRECTION of the PLAYER
    drawItems();
    dispStats();
    if (dead == 1) doDeath();
    drawNames();
    gfx_SwapDraw();
  } while (kb_Data[6] != kb_Clear);
  gfx_End();
  return 0;
}