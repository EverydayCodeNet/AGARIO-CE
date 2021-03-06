#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <tice.h>
#include <keypadc.h>
#include <graphx.h>

kb_key_t key;
void handleKeys(kb_key_t keyLocal);

typedef struct {
  int x;
  int y;
  //radius
  unsigned int i;
  uint8_t dir;
  uint8_t color;
  uint8_t speed;
  uint8_t foodDistance;
  unsigned int foodIdx;
  uint8_t eDistance;
  uint8_t eIdx;
} player_t;

typedef struct {
  int x;
  int y;
  unsigned int i;
  uint8_t dir;
  uint8_t speed;
  uint8_t parentIdx;
  uint8_t parentColor;
  //if clone pos is empty in the array, fill it
  uint8_t filled;
} clone_t;

typedef struct {
  int x;
  int y;
  uint8_t color;
} food_t;

uint8_t maxPlayers = 8;
int foodCount = 14;
int cloneCount = 4;
uint8_t foodConstant = 1;

//largest player size
int topI;
uint8_t dead = 0;

//entity count is array size - 1
player_t players[9];
food_t arrFood[15];
clone_t clones[5];

//range for new player (avg,biggest)
//could be used to respawn
void createPlayers() {
  int idx;
  for (idx = 0; idx < maxPlayers; idx++) {
    player_t* player = &(players[idx]);
    player->color = randInt(1,224);
    if (idx == 0) {
      //player->i = randInt(8,15);
      player->i = 20;
      player->x = 160;
      player->y = 120;
      player->speed = 50 / player->i;
    } else {
      player->x = randInt(-160,480);
      player->y = randInt(-120,360);
      player->i = randInt(10,15);
      player->speed = 50 / player->i;
      //presets to encourage seeking
      player->foodDistance = 50;
      player->eDistance = 100;
    }
    player->dir = randInt(1, 8);
    gfx_SetColor(player->color);
    gfx_FillCircle(player->x, player->y, player->i);
  }
}

void createFood() {
    int idx;
    uint8_t randColor;
    for (idx = 0; idx < foodCount; idx++) {
      food_t* food = &(arrFood[idx]);
      randColor = randInt(1,224);
      gfx_SetColor(randColor);
      food->color = randColor;
      food->x = randInt(-160,480);
      food->y = randInt(-120,360);
      gfx_FillRectangle(food->x,food->y,2,2);
    }
}

void mergeObj(int cloneIdx) {
  player_t* player = &(players[0]);
  clone_t* clone = &(clones[cloneIdx]);
  player->i = player->i + 0.75 * clone->i;
  if (player->i <= 50) player->speed = 50 / player->i;
  if (player->i > 51) player->speed = 1;
  //remove clone
  clone->i = 0;
  clone->filled = 0;
}

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

//copy clone attributes to dead player and remove clone
void shiftClone(int idx) {
  player_t* player = &(players[0]);
  clone_t* clone = &(clones[idx]);
  player->i = clone->i;
  player->speed = 50 / player->i;
  clone->i = 0;
}

//respawn function to eliminate repeat code?
void rescaleObj(int idx, int otherIdx, int type) {
  //type 0 - player on player - player1 is bigger
  //type 1 - clone eating player
  //type 2 - player eating clone
  uint8_t cloneIdx;
  if (type == 0) {
    player_t* player = &(players[idx]);
    player_t* otherPlayer = &(players[otherIdx]);
    if (otherIdx == 0 && otherPlayer->i < player->i) {
      dead = 1;
    } else {
      //place players back on the map
      otherPlayer->x = randInt(-310, 310);
      otherPlayer->y = randInt(-230, 230);
      //range(avg,top player)
      otherPlayer->i = randInt(10,15);
      otherPlayer->dir = randInt(1, 8);
      otherPlayer->color = randInt(1,224);
      //slowest speed is 1
      if (otherPlayer->i <= 50) otherPlayer->speed = 50 / player->i;
      if (otherPlayer->i > 51) otherPlayer->speed = 1;
      gfx_SetColor(otherPlayer->color);
      gfx_FillCircle(otherPlayer->x, otherPlayer->y, otherPlayer->i);
    }
    player->i = player->i + 0.25 * otherPlayer->i;
  } else if (type == 1) {
    player_t* player = &(players[idx]);
    clone_t* clone = &(clones[otherIdx]);
    clone->i = clone->i + 0.25 * player->i;
    if (clone->i <= 50) clone->speed = 50 / clone->i;
    if (clone->i > 51) clone->speed = 1;
    //40px off the edges
    player->x = randInt(-120,440);
    player->y = randInt(-200,320);
    player->i = randInt(10,15);
    player->dir = randInt(1, 8);
    player->color = randInt(1,224);
    player->speed = 50 / player->i;
  } else if (type == 2 && idx != 0) {
    player_t* player = &(players[idx]);
    clone_t* clone = &(clones[otherIdx]);
    player->i = player->i + 0.25 * clone->i;
    if (player->i <= 50) player->speed = 50 / player->i;
    if (player->i > 51) player->speed = 1;
    clone->i = 0;
    clone->filled = 0;
  }
}

int getDistance(int idx, int otherIdx, int type) {
  player_t player = players[idx];
  player_t otherPlayer = players[otherIdx];
  clone_t clone = clones[otherIdx];

  int x1 = player.x;
  int y1 = player.y;
  int x2;
  int y2;
  int xsqr,ysqr;
  int distance;

  if (type == 1) {
    x2 = clone.x;
    y2 = clone.y;
  } else {
    x2 = otherPlayer.x;
    y2 = otherPlayer.y; 
  } 

  xsqr = (x2-x1) * (x2-x1);
  ysqr = (y2-y1) * (y2-y1);

  //distance formula
  distance = sqrt(xsqr + ysqr);
  return distance;
}

//find nearest food
void getFoodDistance(int idx, int otherIdx) {
  player_t* player = &(players[idx]);
  food_t food = arrFood[otherIdx];
  
  int x1,x2,y1,y2,xsqr,ysqr,distance;
  x1 = player->x;
  y1 = player->y;
  x2 = food.x;
  y2 = food.y;
  xsqr = (x2-x1) * (x2-x1);
  ysqr = (y2-y1) * (y2-y1);
  distance = sqrt(xsqr + ysqr);

  if (distance < player->foodDistance && player->foodDistance != 0 ) {
    player->foodDistance = distance;
    player->foodIdx = otherIdx;
  }
}

void findEnemy() {
  uint8_t idx;
  uint8_t otherIdx;
  unsigned int distance;
  for (idx = 0; idx < maxPlayers; idx++) {
    player_t* player = &(players[idx]);
    for (otherIdx = 0; otherIdx < maxPlayers; otherIdx++) {
      //make sure the player doesn't make itself the enemy
      if (idx != otherIdx) {
        distance = getDistance(idx,otherIdx,0);
          if (distance < player->eDistance && idx != otherIdx) {
          player->eDistance = distance;
          player->eIdx = otherIdx;
        }
      }
    }
    for (otherIdx = 0; otherIdx < foodCount; otherIdx++) {
      food_t food = arrFood[otherIdx];
      getFoodDistance(idx,otherIdx);
    }
  }
}

void checkPlayers() {
  int idx;
  int otherIdx;
  //player eating player
  for (idx = 0; idx < maxPlayers; idx++) {
    player_t player = players[idx];
    if (player.i > topI) topI = player.i;
    for (otherIdx = 0; otherIdx < maxPlayers; otherIdx++) {
      player_t otherPlayer = players[otherIdx];
      //check if player size is big enough
      if (player.i > otherPlayer.i) {
        if (getDistance(idx,otherIdx,0) < player.i + otherPlayer.i) {
          //player eating player
          rescaleObj(idx,otherIdx,0);
        }
      }
    }
    for (otherIdx = 0; otherIdx < cloneCount; otherIdx++) {
      clone_t clone = clones[otherIdx];
      if (getDistance(idx,otherIdx,1) < player.i + clone.i) {
        //only type 3 if idx of clone is not the child
        if (clone.parentIdx == idx) {
          mergeObj(otherIdx);
        } else if (getDistance(idx,otherIdx,1) < player.i + clone.i && clone.parentIdx !=idx) {
          //other player is absorbing clone even though it is smaller
          if (player.i < clone.i) rescaleObj(idx,otherIdx,1);
          if (player.i > clone.i) rescaleObj(idx,otherIdx,2);
        }
      }
    }
  }
}

void handleFood() {
  uint8_t idx;
  uint8_t otherIdx;
  
  for (idx = 0; idx < foodCount; idx++) {
    food_t* food = &(arrFood[idx]);  
    for (otherIdx = 0; otherIdx < cloneCount; otherIdx++) {
      clone_t* clone = &(clones[otherIdx]);
      if (food->y > clone->y - clone->i && food->y < clone->y + clone->i 
      && food->x > clone->x - clone->i && food->x < clone->x + clone->i) {
        clone->i++;
        if (clone->i <= 50) clone->speed = 50 / clone->i;
        if (clone->i > 51) clone->speed = 1;
        food->x = randInt(-120,440);
        food->y = randInt(-200,320);
      }
    }
    for (otherIdx = 0; otherIdx < maxPlayers; otherIdx++) {
      player_t* player = &(players[otherIdx]);
      if (food->y > player->y  - player->i && food->y < player->y + player->i 
      && food->x > player->x - player->i && food->x < player->x + player->i) {
        player->i++;
        if (player->i <= 50) player->speed = 50 / player->i;
        if (player->i > 51) player->speed = 1;
        food->x = randInt(-120,440);
        food->y = randInt(-200,320);
      }
    }
  }
}

void movePlayers() {
  unsigned int idx;
  for (idx = 1; idx < maxPlayers; idx++) {
    player_t* player = &(players[idx]);
    player_t enemy = players[player->eIdx];
    if (player->i > 1.5 * enemy.i && player->eDistance < player->foodDistance) {
      if (player->x > enemy.x) player->x = player->x - player->speed;
      if (player->x < enemy.x) player->x = player->x + player->speed;
      if (player->y < enemy.y) player->y = player->y + player->speed;
      if (player->y > enemy.y) player->y = player->y - player->speed;
    } else {
      //eat nearest food
      food_t food = arrFood[player->foodIdx];
      if (food.x > player->x) player->x = player->x + player->speed;
      if (food.x < player->x) player->x = player->x - player->speed;
      if (food.y < player->y) player->y = player->y - player->speed;
      if (food.y > player->y) player->y = player->y + player->speed;
    }
  }
}

void moveClones() {
  int idx;
  int cloneIdx;
  for (cloneIdx = 0; cloneIdx < cloneCount; cloneIdx++) {
    clone_t* clone = &(clones[cloneIdx]);
    if (clone->filled == 1) {
      player_t player = players[clone->parentIdx];
      if (clone->x > player.x) clone-> x = clone->x - clone->speed;
      if (clone->x < player.x) clone-> x = clone->x + clone->speed;;
      if (clone->y < 120) clone-> y = clone->y + clone->speed;
      if (clone->y > player.y) clone-> y = clone->y - clone->speed;
      if (getDistance(0,cloneIdx,1) < player.i + clone->i) {
        mergeObj(cloneIdx);
      }
    }
  }
}

void createClones(int cloneIdx) {
  player_t* player = &(players[0]);
  if (player->i > 9) {
    clone_t* clone = &(clones[cloneIdx]);
    if (clone->filled == 0) {
      player->i = player->i / 2;
      clone->i = player->i;
      clone->parentColor = player->color;
      clone->dir = player->dir;
      player->speed = 50 / player->i;
      clone->speed = player->speed;
      //send clone in direction of player
      if (player->dir == 1) {
        clone->y = randInt(30,70);
        clone->x = player->x;
      } else if (player->dir == 2) {
        clone->x = randInt(210,260);
        clone->y = randInt(30,70); 
      } else if (player->dir == 3) {
        clone->x = randInt(210,260); 
        clone->y = player->y;
      } else if (player->dir == 4) {
        clone->x = randInt(210,260);
        clone->y = randInt(170,210);
      } else if (player->dir == 5) {
        clone->x = player->x;
        clone->y = randInt(170,210); 
      } else if (player->dir == 6) {
        clone->x = randInt(60,110);
        clone->y = randInt(170,210); 
      } else if (player->dir == 7) {
        clone->x = randInt(60,110);
        clone->y = player->y; 
      } else if (player->dir == 8) {
        clone->x = randInt(60,110);
        clone->y = randInt(30,70);
      }
      clone->filled = 1;
    }
  }
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

//move food and respawn it when it reaches edge
void moveFood() {
  int idx;
  player_t player = players[0];
  int dir = player.dir;
  for (idx = 0; idx < foodCount; idx++) {
    food_t* food = &(arrFood[idx]);
    if (dir == 1) {
      food->y = food->y + player.speed;
    } else if (dir == 2) {
      food->x = food->x - player.speed;
      food->y = food->y + player.speed;
    } else if (dir == 3) {
      food->x = food->x - player.speed;
    } else if (dir == 4) {
      food->x = food->x - player.speed;
      food->y = food->y - player.speed;
    } else if (dir == 5) {
      food->y = food->y - player.speed;
    } else if (dir == 6) {
      food->x = food->x + player.speed;
      food->y = food->y - player.speed;
    } else if (dir == 7) {
      food->x = food->x + player.speed;
    } else if (dir == 8) {
      food->x = food->x + player.speed;
      food->y = food->y + player.speed;
    }
    //respawn -40px
    if (food->x > 480 || food->x < -160) food->x = randInt(-120,440);
    if (food->y > 360 || food->y < -120) food->y = randInt(-200,320);
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
  gfx_SetTextScale(1.5,1.5);
  gfx_PrintStringXY("Direction: ",10,10);
  if (player.dir == 1) gfx_PrintString("UP");
  if (player.dir == 3) gfx_PrintString("RIGHT");
  if (player.dir == 5) gfx_PrintString("DOWN");
  if (player.dir == 7) gfx_PrintString("LEFT");
  
  gfx_PrintStringXY("Largest Player: ",10,20);
  gfx_PrintInt(topI,1);
  gfx_PrintStringXY("Your Size: ",10,30);
  gfx_PrintInt(player.i,1);
}

void startGame() {
  srand(rtc_Time());
  createFood();
  clearPlayers();
  createPlayers();
  findEnemy();
}

void main(void) {
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
    drawItems();
    dispStats();
    if (dead == 1) doDeath();
    drawNames();
    gfx_SwapDraw();
  } while (kb_Data[6] != kb_Clear);
  gfx_End();
}