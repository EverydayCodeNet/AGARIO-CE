#include <stdio.h>
#include <stdlib.h>
#include <players.h>

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
  for (int idx = 0; idx < maxPlayers; idx++) {
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
  }
}

void createFood() {
    int idx;
    uint8_t randColor;
    for (idx = 0; idx < foodCount; idx++) {
      food_t* food = &(arrFood[idx]);
      randColor = randInt(1,224);
      food->color = randColor;
      food->x = randInt(-160,480);
      food->y = randInt(-120,360);
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
    }
    player->i = player->i + 0.25 * otherPlayer->i;
  } else if (type == 1) {
    player_t* player = &(players[idx]);
    clone_t* clone = &(clones[otherIdx]);
    clone->i = clone->i + 0.25 * player->i;
    if (clone->i <= 50) clone->speed = 50 / clone->i;
    if (clone->i > 51) clone->speed = 1;
    // 40px off the edges
    // remove magic numbers
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
  int x2, y2;
  int xsqr,ysqr;

  if (type == 1) {
    x2 = clone.x;
    y2 = clone.y;
  } else {
    x2 = otherPlayer.x;
    y2 = otherPlayer.y; 
  } 

  return sqrt(pow((x2 - x1),2) + pow((y2 - y1),2));
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

  distance = sqrt(pow((x2 - x1),2) + pow((y2 - y1),2));

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
  for (int idx = 1; idx < maxPlayers; idx++) {
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