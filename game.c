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
    tiles[tilesLeft-1] = -1;
}

void createPlateau(){
    printf("Création du plateau\n");
    plateau = smalloc(PLATEAU_LENGTH * sizeof(int));
    for(int i = 0; i < PLATEAU_LENGTH; i++){
        plateau[i] = 0;
    }
}

void readAndCreateTilesTab(char* filename){
    disableRandomDraw();
    int file = sopen(filename, O_RDONLY, 0);
    char** lines = readFileToTable(file);
    if(lines == NULL) return;
    tiles = smalloc(tilesLeft * sizeof(int));
    int index = 0;
    for(int i = 0; i < tilesLeft; i++){
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
    if(tilesLeft == 0){
        freeTiles();
        createTilesTab();
    }
    int index = doRandomDraw ? randomIntBetween(0, tilesLeft) : 0;
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
    position--;
    if(position == -1) return false;
    if(plateau[position] != 0){
        // passage à droite
        for(int i = position; i < PLATEAU_LENGTH-1; i++){
            if(plateau[i] == 0){
                plateau[i] = tile;
                return true;
            }
        }
        // passage à gauche
        for(int i = 0; i >= 0; i++){
            if(plateau[i] == 0){
                plateau[i] = tile;
                return true;
            }
        }  
    } 
    plateau[position] = tile; 
    return true;
} 

int scoreForStreak(int streak) {
    if(streak <= 1) return 0;
    if(streak == 2) return 1;
    if(streak == 3) return 3;
    if(streak == 4) return 5;
    if(streak == 5) return 7;
    if(streak == 6) return 9;
    if(streak == 7) return 11;
    if(streak == 8) return 15;
    if(streak == 9) return 20;
    if(streak == 10) return 25;
    if(streak == 11) return 30;
    if(streak == 12) return 35;
    if(streak == 13) return 40;
    if(streak == 14) return 50;
    if(streak == 15) return 60;
    if(streak == 16) return 70;
    if(streak == 17) return 85;
    if(streak == 18) return 100;
    if(streak == 19) return 150;
    return 300;
}

int calculateScore(){
    int score = 0;
    int streak = 1;

    for(int i = 1; i < PLATEAU_LENGTH; i++){
        if(plateau[i] >= plateau[i-1] || plateau[i] == -1) {
            streak++;
        } else {
            score += scoreForStreak(streak);
            streak = 1;
        }
    }
    score += scoreForStreak(streak);
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

void freeTiles(){
    if(tiles != NULL) free(tiles);

} 

void freePlateau(){
    if(plateau != NULL) free(plateau);
}

void displayPlateau(){
    for(int i = 0; i < PLATEAU_LENGTH; i++){
        printf("%d ", plateau[i]);
    }
    printf("\n");
}
