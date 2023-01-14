#include <stdio.h>
#include <stdlib.h>
#include <players.h>
#include <draw.h>

void clearPlayers() {
  gfx_FillScreen(255);
}

void drawItems() {
  int scaledRadius;
  for (int idx = 0; idx < foodCount; idx++) {
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

// pass individual player instead of entire array in the future
void drawDeathScreen() {
  player_t player = players[0];
  gfx_SetDrawBuffer();
  gfx_FillScreen(224);
  gfx_SetTextScale(2,2);
  gfx_PrintStringXY("YOU DIED", 160 - gfx_GetStringWidth("YOU DIED") / 2,95);
  gfx_SetTextScale(1,1);
  gfx_SetColor(255);
  gfx_PrintStringXY("Press any key to continue.",160 - gfx_GetStringWidth("Press any key to continue.") / 2,117);
  gfx_PrintStringXY("Score: ", 160 - gfx_GetStringWidth("Score:   ") / 2,130);
  gfx_PrintInt(player.i,1);
  gfx_SwapDraw();
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