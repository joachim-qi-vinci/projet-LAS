#ifndef _GAME_H_
#define _GAME_H_

#include <stdbool.h>
#include "messages.h"  
/**
 * Create the table with chars from 1 to 30 where 11 to 19 is
 * present two times
**/
void createTilesTab();

/**
 * Create the game plateau
**/
void createPlateau();

/**
 * Return a random element in the table and remove it
**/
int drawTile();

/**
 * Place a tile in the table
**/
bool placeTile(int position, int tile);

/**
 * Calculate the score of the plateau
**/
int calculateScore();

/**
 * Send score to server
**/
void sendScore();

/**
 * Sort the scores
 * playerTab is the adress of a table
 * playertab is sorted
**/
void sortTabScores(Player* playerTab);

/**
 * Free the tiles and plateau
**/
void closeGame();
#endif