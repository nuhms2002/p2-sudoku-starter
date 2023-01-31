// Sudoku puzzle verifier and solver


#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
//#include <string.h>

// takes puzzle size and grid[][] representing sudoku puzzle
// and tow booleans to be assigned: complete and valid.
// row-0 and column-0 is ignored for convenience, so a 9x9 puzzle
// has grid[1][1] as the top-left element and grid[9]9] as bottom right
// A puzzle is complete if it can be completed with no 0s in it
// If complete, a puzzle is valid if all rows/columns/boxes have numbers from 1
// to psize For incomplete puzzles, we cannot say anything about validity
int size;
bool legal = true;
bool done = true;
int **copyOfGrid;

typedef struct {
  int row;
  int column;
  int subRowSize;
} parameters; 

void* rowCheck(void* arg);
void* colCheck(void* arg);
void* subGridCheck(void* arg);

// bool verifyPuzzleComplete(int** puzzle, int size) {
//   bool complete = true;
//   for (int row = 1; row <= size; row++) {
//     for (int col = 1; col <= size; col++) {
//       if (puzzle[row][col] == 0) {
//         complete = false;
//         break;
//       }
//     }
//   }
//   return complete;
// }

void checkPuzzle(int psize, int **grid, bool *complete, bool *valid) {
  // YOUR CODE GOES HERE and in HELPER FUNCTIONS
  size = psize;
  copyOfGrid = grid;

  pthread_t rowThreads[psize + 1];
  pthread_t columnThreads[psize + 1];
  pthread_t subGridThreads[psize + 1];
  
  for(int i=1; i <= psize; i++){
      parameters *curr = (parameters *) malloc(sizeof(parameters));
      curr->row = i;
      curr->column = 1;
      curr->subRowSize = psize;
      pthread_create(&rowThreads[i], NULL, rowCheck, curr);

      curr = (parameters *) malloc(sizeof(parameters));
      curr->row = 1;
      curr->column = i;
      curr->subRowSize = psize;
      pthread_create(&columnThreads[i], NULL, colCheck, curr);
  }

  int z = 1;
  int stSize = sqrt(psize);
  for(int y = 1; y <= psize; y += stSize){
    for(int x = 1; x <= psize; x += stSize){
      parameters *curr = (parameters *) malloc(sizeof(parameters));
      curr->row = y;
      curr->column = x;
      curr->subRowSize = stSize;
      pthread_create(&subGridThreads[z], NULL, subGridCheck, curr);
      z++;
    }
  }
  for(int i=1; i<=psize; i++){
    pthread_join(rowThreads[i], NULL);
    pthread_join(columnThreads[i], NULL);
    pthread_join(subGridThreads[i], NULL);
  }

  *valid = legal;
  *complete = done;
}

  void* colCheck(void* arg){
    parameters * curr = (parameters *) arg;
    int colValues[size + 1]; //array to hold sorted column
    memset(colValues, 0, sizeof(int) * (size + 1));
    for (int i = 1; i <= size; i++) {
      if (done == false || legal == false){
        return 0;
      }
      //value is a 0
      if (copyOfGrid[i][curr->column] == 0) {
        done = false;
        return 0;
      }
      //value has been seen before
      if (colValues[copyOfGrid[i][curr->column]] == 1) {
        legal = false;
        printf("col inval");
        fflush(stdout);
        return 0;
      } else {
        colValues[copyOfGrid[i][curr->column]] = 1;
      }
    }
    return 0;
  }
  
  void* rowCheck(void* arg){
    parameters * curr = (parameters *) arg;
    //array is to keep track of values that have been seen
    //rowValues[7] being = 1 means we've seen the value before
    int rowValues[size + 1]; //array to hold sorted column
    memset(rowValues, 0, sizeof(int) * (size + 1));
    for (int i = 1; i <= size; i++) {
      if (done == false || legal == false){
        return 0;
      }
      //value is a 0, means puzzle not complete
      if (copyOfGrid[curr->row][i] == 0) {
        done = false;
        return 0;
      }
      //value has been seen before because == 1
      if (rowValues[copyOfGrid[curr->row][i]] == 1) {
        legal = false;
        printf("row inval");
        fflush(stdout);
        return 0;
      } else {
        //if we have not seen the value set it to 1
        rowValues[copyOfGrid[curr->row][i]] = 1;
      }
    }
    return 0;
  }

  void* subGridCheck(void* arg){
    parameters *curr = (parameters*) arg;
    int s = size+1;
    int visited[s];
    memset(visited, 0, sizeof(int) * s);
    for(int i = curr->row; i <= curr->row + curr->subRowSize; i++){
      for(int j = curr->column; j <= curr->column + curr->subRowSize; j++){
        if (done == false || legal == false){
          return 0;
        }
        if (copyOfGrid[i][j] == 0) {
          done = false;
          return 0;
        }
        if(visited[copyOfGrid[i][j]] != 0){
          legal = false;
          printf("box inval [%d][%d]", i, j);
          fflush(stdout);
          return 0;
        } else {
          visited[copyOfGrid[i][j]] = 1;
        }
      }
    }
  return 0;
}


// takes filename and pointer to grid[][]
// returns size of Sudoku puzzle and fills grid
int readSudokuPuzzle(char *filename, int ***grid) {
  FILE *fp = fopen(filename, "r");
  if (fp == NULL) {
    printf("Could not open file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  int psize;
  fscanf(fp, "%d", &psize);
  int **agrid = (int **)malloc((psize + 1) * sizeof(int *));
  for (int row = 1; row <= psize; row++) {
    agrid[row] = (int *)malloc((psize + 1) * sizeof(int));
    for (int col = 1; col <= psize; col++) {
      fscanf(fp, "%d", &agrid[row][col]);
    }
  }
  fclose(fp);
  *grid = agrid;
  return psize;
}

// takes puzzle size and grid[][]
// prints the puzzle
void printSudokuPuzzle(int psize, int **grid) {
  printf("%d\n", psize);
  for (int row = 1; row <= psize; row++) {
    for (int col = 1; col <= psize; col++) {
      printf("%d ", grid[row][col]);
    }
    printf("\n");
  }
  printf("\n");
}

// takes puzzle size and grid[][]
// frees the memory allocated
void deleteSudokuPuzzle(int psize, int **grid) {
  for (int row = 1; row <= psize; row++) {
    free(grid[row]);
  }
  free(grid);
}

// expects file name of the puzzle as argument in command line
int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage: ./sudoku puzzle.txt\n");
    return EXIT_FAILURE;
  }
  // grid is a 2D array
  int **grid = NULL;
  // find grid size and fill grid
  int sudokuSize = readSudokuPuzzle(argv[1], &grid);
  bool valid = false;
  bool complete = false;
  checkPuzzle(sudokuSize, grid, &complete, &valid);
  printf("Complete puzzle? ");
  printf(complete ? "true\n" : "false\n");
  if (complete) {
    printf("Valid puzzle? ");
    printf(valid ? "true\n" : "false\n");
  }
  printSudokuPuzzle(sudokuSize, grid);
  deleteSudokuPuzzle(sudokuSize, grid);
  return EXIT_SUCCESS;
}
