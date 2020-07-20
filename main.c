#include <mpi.h>
#include <stdio.h>
#include <string.h>

const int MASTER = 0;
const int BUFFER_SIZE = 100;

// A utility function to find min of two integers 
int min(int a, int b) 
{ return (a < b)? a: b; } 
  
// A utility function to find min of three integers 
int min3(int a, int b, int c) 
{ return min(min(a, b), c);} 
  
// A utility function to find max of two integers 
int max(int a, int b) 
{ return (a > b)? a: b; } 

void sequential_lcs(char *s1, char *s2) {
    int len_s1 = strlen(s1) + 1;
    int len_s2 = strlen(s2) + 1;

    int dp[len_s1][len_s2];

    for (int line=1; line<len_s1+len_s2; line++) {
        int start_col =  max(0, line-len_s1); 
        int count = min3(line, (len_s2-start_col), len_s1); 
  
        for (int j=0; j<count; j++) {
            int row = min(len_s1, line)-j-1;
            int col = start_col+j;

            if (row==0 || col==0)
                dp[row][col] = 0;
            else if (s1[row - 1] == s2[col - 1]) 
                dp[row][col] = dp[row-1][col-1] + 1;
            else 
                dp[row][col] = max(dp[row][col-1], dp[row-1][col]);
        }
    }

    for (int i=0; i<len_s1; i++) 
    { 
        for (int j=0; j<len_s2; j++) 
            printf("%5d ", dp[i][j]); 
        printf("\n"); 
    } 
}

int main(int argc, char** argv) {

    char s1[BUFFER_SIZE], s2[BUFFER_SIZE];

    strcpy(s1, "TAGTCACG");
    strcpy(s2, "AGACTGTC");

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

    // Broadcast input strings to all processes
	MPI_Bcast(s1, BUFFER_SIZE, MPI_CHAR, MASTER, MPI_COMM_WORLD);
	MPI_Bcast(s2, BUFFER_SIZE, MPI_CHAR, MASTER, MPI_COMM_WORLD);

    int len_s1 = strlen(s1) + 1;
    int len_s2 = strlen(s2) + 1;

    int block_first, block_last, row, col;
    MPI_Status status;
    MPI_Request request1, request2;
    double start_time, finish_time, duration;

    int dp[len_s1][len_s2];

    for(int i=0; i<len_s1; i++)
        for(int j=0; j<len_s2; j++)
            dp[i][j] = -1;

    for (int line=1; line<len_s1+len_s2; line++) {
        int start_col =  max(0, line-len_s1); 
        int count = min3(line, (len_s2-start_col), len_s1); 

        int block_len = count / size;
        if (count % size)
            block_len++;

        int start = block_len*rank;
        int end = block_len*(rank+1)-1;
  
        for (int j=start; j<=end && j<count; j++) {
            row = min(len_s1, line)-j-1;
            col = start_col+j;

            if (row==0 || col==0)
                dp[row][col] = 0;
            else if (s1[row - 1] == s2[col - 1]) 
                dp[row][col] = dp[row-1][col-1] + 1;
            else 
                dp[row][col] = max(dp[row][col-1], dp[row-1][col]); 

            if (j == start && rank > 0) {
                // printf("line %d sending from %d to %d \n", line, rank, rank-1);
                MPI_Send(&dp[row][col], 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD);
            }
            if (j == end && rank < size-1) {
                // printf("line %d sending from %d to %d \n", line, rank, rank+1);
                MPI_Send(&dp[row][col], 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD);
            }
        }

        int prev_index = start - 1;
        if (prev_index >= 0 && prev_index < count) {
            row = min(len_s1, line)-prev_index-1;
            col = start_col+prev_index;
            // printf("line %d receive from %d to %d \n", line, rank-1, rank);
            MPI_Recv(&dp[row][col], 1, MPI_INT, rank-1, 1, MPI_COMM_WORLD, &status);
        }

        int next_index = end + 1;
        if (next_index >= 0 && next_index < count) {
            row = min(len_s1, line)-next_index-1;
            col = start_col+next_index;
            // printf("line %d receive from %d to %d \n", line, rank+1, rank);
            MPI_Recv(&dp[row][col], 1, MPI_INT, rank+1, 1, MPI_COMM_WORLD, &status);
        }
    }

    if (rank == 0) {
        for (int i=0; i<len_s1; i++) { 
            for (int j=0; j<len_s2; j++) 
                printf("%5d ", dp[i][j]); 
            printf("\n"); 
        } 
        printf("\n"); 
    }

    MPI_Finalize();
}