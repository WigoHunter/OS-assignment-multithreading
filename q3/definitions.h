#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#define DEBUG 0


/*---------Do Not Change Anything Below This Line---------*/
//Job ID
#define SKELETON  0 // 5 sec
#define ENGINE    1 // 4 sec
#define CHASSIS   2 // 3 sec
#define BODY      3 // 4 sec
#define WINDOW    4 // 7 sec
#define TIRE      5 // 8 sec
#define BATTERY   6 // 3 sec
#define CAR       7 // 6 sec

/*-----Production time for each item-----*/
// Phase 1
#define TIME_SKELETON 5
#define TIME_ENGINE   4
#define TIME_CHASSIS  3
#define TIME_BODY     4

// Phase 2
#define TIME_WINDOW   1
#define TIME_TIRE     2
#define TIME_BATTERY  3
#define TIME_CAR      6

