#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

const int MASTER = 0;

// A utility function to find min of two integers 
int min(int a, int b) 
{ return (a < b)? a: b; } 
  
// A utility function to find min of three integers 
int min3(int a, int b, int c) 
{ return min(min(a, b), c);} 
  
// A utility function to find max of two integers 
int max(int a, int b) 
{ return (a > b)? a: b; } 

void print_lcs(int rows, int cols, char traceback[rows][cols], char *s1, int i, int j) {
    if (i==0 || j==0) return;
    switch(traceback[i][j]) {
        case 0:
            print_lcs(rows, cols, traceback, s1, i-1, j-1);
            printf("%c", s1[i-1]);
            break;
        case 1:
            print_lcs(rows, cols, traceback, s1, i, j-1);
            break;
        case 2:
            print_lcs(rows, cols, traceback, s1, i-1, j);
            break;
    }
}

void print_traceback(int rows, int cols, char traceback[rows][cols], char *s1, char *s2) {
    for (int i=0; i<rows; i++) { 
        for (int j=0; j<cols; j++) {
            switch (traceback[i][j]) {
                case 0:
                    printf("D");
                    break;
                case 1:
                    printf("L");
                    break;
                case 2:
                    printf("R");
                    break;
                default:
                    printf("0");
                    break;
            }
        }
        printf("\n"); 
    } 
    printf("\n"); 
    printf("longest common subsequence = ");
    print_lcs(rows, cols, traceback, s1, rows-1, cols-1);
    printf("\n"); 
}

void print_matrix(int rows, int cols, int matrix[rows][cols]) {
    for (int i=0; i<rows; i++) 
    { 
        for (int j=0; j<cols; j++) 
            printf("%2d ", matrix[i][j]); 
        printf("\n"); 
    } 
    printf("\n"); 
}

int sequential_lcs(char *s1, char *s2) {
    int rows = strlen(s1) + 1;
    int cols = strlen(s2) + 1;

    int dp[3][min(rows, cols)];
    char traceback[rows][cols];

    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++)
            traceback[i][j] = -1;

    for (int line=1; line<rows+cols; line++) {
        int curr_line = line % 3;
        int prev_line = (line-1) % 3;
        int prev_prev_line = (line-2) % 3;

        int start_col =  max(0, line-rows); 
        int count = min3(line, (cols-start_col), rows); 
  
        for (int j=0; j<count; j++) {
            int row = min(rows, line)-j-1;
            int col = start_col+j;

            if (row==0 || col==0) {
                dp[curr_line][col] = 0;
            }
            else if (s1[row - 1] == s2[col - 1]) {
                int upper_left = dp[prev_prev_line][col-1];
                dp[curr_line][col] = upper_left + 1;
                traceback[row][col] = 0;
            }
            else {
                int left = dp[prev_line][col-1];
                int up = dp[prev_line][col];
                if (left > up) {
                    dp[curr_line][col] = left;
                    traceback[row][col] = 1;
                } else {
                    dp[curr_line][col] = up;
                    traceback[row][col] = 2;
                }
            }
        }
    }

    print_traceback(rows, cols, traceback, s1, s2);
    return dp[(rows+cols-1) % 3][cols-1];
}

int lcs_parallel(char* s1, char *s2, int len_s1, int len_s2, int rank, int size) {
    int rows = len_s1 + 1;
    int cols = len_s2 + 1;

    int row, col;
    MPI_Status status;

    int dp[rows][cols];

    for (int line=1; line<rows+cols; line++) {
        int start_col =  max(0, line-rows); 
        int count = min3(line, (cols-start_col), rows); 

        int start, end;
        if (count <= size) {
            start = rank;
            end = min(rank, count-1);
        } else {
            float block_len = (float)count / size;
            start = round(block_len*rank);
            end = round(block_len*(rank+1))-1;
        }
  
        for (int j=start; j<=end; j++) {
            row = min(rows, line)-j-1;
            col = start_col+j;

            if (row==0 || col==0) {
                dp[row][col] = 0;
            }
            else if (s1[row - 1] == s2[col - 1]) {
                if(dp[row-1][col-1] == -1)
                    printf("ERROR row=%d col=%d rank=%d line=%d\n", row-1, col-1, rank, line);
                dp[row][col] = dp[row-1][col-1] + 1;
            }
            else {
                if(dp[row-1][col] == -1)
                    printf("ERROR row=%d col=%d rank=%d line=%d\n", row-1, col, rank, line);
                if(dp[row][col-1] == -1)
                    printf("ERROR row=%d col=%d rank=%d line=%d\n", row, col-1, rank, line);

                dp[row][col] = max(dp[row][col-1], dp[row-1][col]); 
            }

            if (j == start && rank > 0) {
                MPI_Send(&dp[row][col], 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD);
            }
            if (j == end && rank < size-1) {
                MPI_Send(&dp[row][col], 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD);
            }
        }

        int prev_index = start - 1;
        if (prev_index >= 0 && prev_index < count) {
            row = min(rows, line)-prev_index-1;
            col = start_col+prev_index;
            MPI_Recv(&dp[row][col], 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &status);
        }

        int next_index = end + 1;
        if (next_index >= 0 && next_index < count) {
            row = min(rows, line)-next_index-1;
            col = start_col+next_index;
            MPI_Recv(&dp[row][col], 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &status);
        }
    }

    return dp[rows-1][cols-1];
}

void load_input(char *filename, char **s1ptr, char **s2ptr, int rank) {

    int len_s1, len_s2;
    char *s1, *s2;
    FILE* file;

    if (rank == MASTER) {
        file = fopen(filename, "r");
        fscanf(file, "%d\n%d\n", &len_s1, &len_s2);
    }

    MPI_Bcast(&len_s1, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&len_s2, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

    s1 = malloc(len_s1+1);
    s2 = malloc(len_s2+1);

    if (rank == MASTER) {
        for(int i=0; i<len_s1; i++)
            s1[i] = fgetc(file);
        s1[len_s1] = '\0';
        if (fgetc(file) != '\n') {
            printf("error in input file format");
            exit(1);
        }
        for(int i=0; i<len_s2; i++)
            s2[i] = fgetc(file);
        s2[len_s2] = '\0';
    }

    MPI_Bcast(&s1[0], len_s1, MPI_CHAR, MASTER, MPI_COMM_WORLD);
    MPI_Bcast(&s2[0], len_s2, MPI_CHAR, MASTER, MPI_COMM_WORLD);

    *s1ptr = s1;
    *s2ptr = s2;
}

int main(int argc, char** argv) {

    MPI_Init(NULL, NULL);

    // Get the number of processes
    int size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Get the rank of the process
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, rank, size);

    char *s1, *s2;
    load_input(argv[1], &s1, &s2, rank);
    int len_s1 = strlen(s1);
    int len_s2 = strlen(s2);

    if (rank == MASTER) {
        printf("s1 = %s\n", s1);
        printf("s2 = %s\n", s2);
        printf("s1 length: %d characters\n", len_s1);
        printf("s2 length: %d characters\n", len_s2);
        printf("\n"); 
    }

    double start_time, finish_time, duration;
    if (rank == MASTER)
        start_time = MPI_Wtime();

    int result = lcs_parallel(s1, s2, len_s1, len_s2, rank, size);

    if (rank == MASTER) {
		finish_time = MPI_Wtime();
		duration = finish_time - start_time;
        printf("\nPARALLEL ALGORITHM\n");
        printf("Parallel completed in %f ms \n", duration*1000);
        printf("Longest common subsequence length: %d \n", result);
        // print_matrix(rows, cols, dp);

        printf("\n\nSEQUENTIAL ALGORITHM\n");
        start_time = MPI_Wtime();
        int lcs_length = sequential_lcs(s1, s2);
        finish_time = MPI_Wtime();
        duration = finish_time - start_time;
        printf("Sequential completed in %f ms \n", duration*1000);
        printf("Longest common subsequence length: %d \n", lcs_length);
    }

    MPI_Finalize();
}