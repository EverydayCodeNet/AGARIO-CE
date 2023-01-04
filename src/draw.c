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