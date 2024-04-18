#ifndef _GAME_H_
#define _GAME_H_

#include <bool.h>

/**
 * Create a table with chars from 1 to 30 where 11 to 19 is
 * present two times
**/
char* createTilesTab();

/**
 * Return a random element in the table and remove it
**/
char* drawTile(char* tilesTab);

/**
 * Place a tile in the table
**/
bool placeTile(int position, int tile);

/**
 * Send score to server
**/
void sendScore(int score);


/**
 * 
**/
Player* sortTabScores();

#endif