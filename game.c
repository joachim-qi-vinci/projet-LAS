#include <stdbool.h>
#include <stdlib.h>  
#include <fcntl.h> 


#include "game.h"
#include "utils_v1.h"


#define PLATEAU_LENGTH 20

int* plateau = NULL;
int* tiles = NULL;
int tilesLeft = 40;

bool doRandomDraw = true;


void disableRandomDraw(){
    doRandomDraw = false;
}
/**
 * Create a table with chars from 1 to 30 where 11 to 19 is
 * present two times
**/
void createTilesTab() {
    tiles = smalloc(tilesLeft * sizeof(int));
   int index = 0;

    for(int i = 1; i <= 10; i++){ 
        tiles[index] = i;
        index++;
    } 

    for(int j = 0; j < 2; j++)
        for(int i = 11; i <= 19; i++){  
            tiles[index] = i;
            index++;
        }

    for(int i = 20; i <= 30; i++){ 
        tiles[index] = i;
        index++;
    }
    tiles[tilesLeft] = -1;
}

void createPlateau(){
    plateau = smalloc(PLATEAU_LENGTH * sizeof(int));
    
}

void readAndCreateTilesTab(char* filename){
    disableRandomDraw();
    int file = sopen(filename, O_RDONLY, 0);
    char** lines = readFileToTable(file);
    if(lines == NULL) return;
    tiles = smalloc(tilesLeft * sizeof(int));
    int index = 0;
    for(int i = 0; lines[i] != NULL; i++){
        tiles[index] = atoi(lines[i]);
        free(lines[i]);
        index++;
    }
    free(lines);
    sclose(file);
}

/**
 * Return a random element in the table and remove it
**/
int drawTile(){
    int index = doRandomDraw ? 0 : randomIntBetween(0, tilesLeft);
    int tile = tiles[index];
    for(int i = index; i < tilesLeft-1; i++){
        tiles[i] = tiles[i+1];  
    }
    tilesLeft--;
    return tile;
}

/**
 * Place a tile in the table
**/
bool placeTile(int position, int tile){
    if(position == -1) return false;
    if(plateau[position] != 0) return false;
    plateau[position] = tile; 
    return true;
} 

int calculateScore(){
    int score = 0;

    int streak = 0;
    for(int i = 0; i < PLATEAU_LENGTH-1; i++){
        if(plateau[i] < plateau[i+1] || plateau[i] == -1) streak++;
        else {
            if(streak <= 1) score += 0;
            if(streak == 2) score += 1;
            if(streak == 3) score += 3;
            if(streak == 4) score += 5;
            if(streak == 5) score += 7;
            if(streak == 6) score += 9;
            if(streak == 7) score += 11;
            if(streak == 8) score += 15;
            if(streak == 9) score += 20;
            if(streak == 10) score += 25;
            if(streak == 11) score += 30;
            if(streak == 12) score += 35;
            if(streak == 13) score += 40;
            if(streak == 14) score += 50;
            if(streak == 15) score += 60;
            if(streak == 16) score += 70;
            if(streak == 17) score += 85;
            if(streak == 18) score += 100;
            if(streak == 19) score += 150;
            if(score >= 20) score += 300;
            streak = 0;
        }
    }
    return score;
}

void sendScore(){
    // TODO: Utiliser network pour envoyer le score
    // int score = calculateScore();
}

void sortTabScore(Player** players, int size){
    for (int i = 0; i < size-1; i++){
        for (int j = 0; j < size-i-1; j++){
          if (players[j]->score < players[j+1]->score){
            Player* temp = players[j];
            players[j] = players[j+1];
            players[j+1] = temp;
            }
        }
    }
}

void closeGame(){
    if(tiles != NULL) free(tiles);
    if(plateau != NULL) free(plateau);
}
