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

void dispStats() {
  player_t player = players[0];
  gfx_SetTextScale(1,1);
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
    // drawGrid();
    drawItems();
    dispStats();
    if (dead == 1) {
      doDeath();
      do {
        drawDeathScreen();
      } while (!os_GetCSC());
      dead = 0;
    }
    drawNames();
    gfx_SwapDraw();
  } while (kb_Data[6] != kb_Clear);
  gfx_End();
  return 0;
}