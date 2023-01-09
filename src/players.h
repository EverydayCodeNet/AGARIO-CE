#ifndef PLAYERS_H
#define PLAYERS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// create additional game struct to store high score, number of players, etc.
typedef struct {
  int x;
  int y;
  //radius
  unsigned int radius;
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
  unsigned int radius;
  uint8_t dir;
  uint8_t speed;
  uint8_t parentIdx;
  uint8_t parentColor;
  // if clone pos is empty in the array, fill it
  uint8_t filled;
  // replace filled attribute with shift array function
} clone_t;

typedef struct {
  int x;
  int y;
  uint8_t color;
} food_t;

void shiftArray(int *arr[], int start, int end);

// player functions
void createPlayers();
void checkPlayers();
void movePlayers();
int getDistance(int idx, int otherIdx, int type);
void findEnemy();

// clone functions
void createClones();
void moveClones();
void mergeObj(int cloneIdx);
void shiftClone(int idx);
void rescaleObj(int idx, int otherIdx, int type);

// food functions
void createFood();
void moveFood();
void getFoodDistance(int idx, int otherIdx);
void handleFood();

#ifdef __cplusplus
}
#endif

#endif