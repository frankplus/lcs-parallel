#include <mpi.h>
#include <stdio.h>
#include <string.h>

const int MASTER = 0;
const int BUFFER_SIZE = 150;
const char* INPUT_FILENAME = "input.txt";

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

void print_info(int rows, int cols, char traceback[rows][cols], char *s1, char *s2, int lcs_length) {

    printf("s1 = %s\n", s1);
    printf("s2 = %s\n", s2);
    printf("\n"); 

    for (int i=0; i<rows; i++) { 
        for (int j=0; j<cols; j++) {
            switch (traceback[i][j]) {
                case 0:
                    printf("\u2b09");
                    break;
                case 1:
                    printf("\u2190");
                    break;
                case 2:
                    printf("\u2191");
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
    printf("lcs length = %d\n", lcs_length); 
}

void sequential_lcs(char *s1, char *s2) {
    int rows = strlen(s1) + 1;
    int cols = strlen(s2) + 1;

    int dp[rows][cols];
    char traceback[rows][cols];

    for (int line=1; line<rows+cols; line++) {
        int start_col =  max(0, line-rows); 
        int count = min3(line, (cols-start_col), rows); 
  
        for (int j=0; j<count; j++) {
            int row = min(rows, line)-j-1;
            int col = start_col+j;

            if (row==0 || col==0) {
                dp[row][col] = 0;
            }
            else if (s1[row - 1] == s2[col - 1]) {
                dp[row][col] = dp[row-1][col-1] + 1;
                traceback[row][col] = 0;
            }
            else {
                dp[row][col] = max(dp[row][col-1], dp[row-1][col]); 
                traceback[row][col] = dp[row][col-1] > dp[row-1][col] ? 1 : 2;
            }
        }
    }

    // print_info(rows, cols, traceback, s1, s2, dp[rows-1][cols-1]);
}

void print_matrix(int rows, int cols, int matrix[rows][cols]) {
    for (int i=0; i<rows; i++) 
    { 
        for (int j=0; j<cols; j++) 
            printf("%3d ", matrix[i][j]); 
        printf("\n"); 
    } 
    printf("\n"); 
}

int main(int argc, char** argv) {

    // load inputs from file
    FILE* file = fopen(INPUT_FILENAME, "r");
    int len_s1, len_s2;
    fscanf(file, "%d\n%d\n", &len_s1, &len_s2);
    char s1[len_s1+1], s2[len_s2+1];
    fscanf(file, "%s\n%s\n", s1, s2);

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

    int rows = strlen(s1) + 1;
    int cols = strlen(s2) + 1;

    int block_first, block_last, row, col;
    MPI_Status status;
    MPI_Request request1, request2;
    double start_time, finish_time, duration;

    if (rank == MASTER)
        start_time = MPI_Wtime();

    int dp[rows][cols];
    char traceback[rows][cols];

    for(int i=0; i<rows; i++)
        for(int j=0; j<cols; j++) {
            traceback[i][j] = -1;
            dp[i][j] = -1;
        }

    for (int line=1; line<rows+cols; line++) {
        int start_col =  max(0, line-rows); 
        int count = min3(line, (cols-start_col), rows); 

        int block_len = count / size;
        if (count % size)
            block_len++;

        int start = block_len*rank;
        int end = block_len*(rank+1)-1;
  
        for (int j=start; j<=end && j<count; j++) {
            row = min(rows, line)-j-1;
            col = start_col+j;

            if (row==0 || col==0) {
                dp[row][col] = 0;
            }
            else if (s1[row - 1] == s2[col - 1]) {
                dp[row][col] = dp[row-1][col-1] + 1;
                traceback[row][col] = 0;
            }
            else {
                dp[row][col] = max(dp[row][col-1], dp[row-1][col]); 
                traceback[row][col] = dp[row][col-1] > dp[row-1][col] ? 1 : 2;
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

    if (rank == MASTER) {
        for (int receive_from=0; receive_from<size; receive_from++) {
            if (receive_from == MASTER) continue;
            char buffer_traceback[rows][cols];
            MPI_Recv(buffer_traceback, rows*cols, MPI_CHAR, receive_from, 1, MPI_COMM_WORLD, &status);

            for (int i=0; i<rows; i++) {
                for (int j=0; j<cols; j++) {
                    if (buffer_traceback[i][j] != -1) {
                        traceback[i][j] = buffer_traceback[i][j];
                    }
                }
            }
        }

        printf("PARALLEL ALGORITHM\n");
		finish_time = MPI_Wtime();
		duration = finish_time - start_time;
        print_info(rows, cols, traceback, s1, s2, dp[rows-1][cols-1]);
        printf("Parallel completed in %f ms \n", duration*1000);

        printf("\n\nSEQUENTIAL ALGORITHM\n");
        start_time = MPI_Wtime();
        sequential_lcs(s1, s2);
        finish_time = MPI_Wtime();
        duration = finish_time - start_time;
        printf("Sequential completed in %f ms \n", duration*1000);
    }
    else {
        MPI_Send(traceback, rows*cols, MPI_CHAR, MASTER, 1, MPI_COMM_WORLD);
    }

    MPI_Finalize();
}