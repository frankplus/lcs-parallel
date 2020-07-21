from math import ceil
ROW = 50
COL = 50
SIZE = 6

draw = ['-', 'X', '.', '*', 'O', '~']

# Utility function to print a matrix 
def printMatrix(matrix) : 
    for i in range(0, ROW) : 
        for j in range(0, COL) : 
            print(draw[matrix[i][j]], end = " ") 
              
        print() 
  
matrix = [ [ 0 for i in range(ROW) ] for j in range(COL) ] 

for rank in range(0,SIZE):
    for line in range(1, (ROW + COL)) : 
        start_col = max(0, line - ROW) 
        count = min(line, (COL - start_col), ROW) 

        # if count <= SIZE:
        #     if rank == 0:
        #         start = 0
        #         end = count
        #     else:
        #         continue
        # else:
        block_len = count / SIZE
        start = int(block_len*rank)
        end = int(block_len*(rank+1))

        # end = min(block_len*(rank+1), count) if rank != SIZE-1 else count

        for j in range(start, end):
            row = min(ROW, line) - j - 1
            col = start_col + j
            matrix[row][col] = rank
            # print(f"row={row} col={col}")

printMatrix(matrix)

def near(x, rank):
    return (x >= rank-1) and (x <= rank+1)

for i in range(1, ROW) : 
    for j in range(1, COL) : 
        if not near(matrix[i-1][j-1], matrix[i][j]):
            print(f"problem 1 row={i} col={j}")
        if not near(matrix[i][j-1], matrix[i][j]):
            print(f"problem 2 row={i} col={j}")
        if not near(matrix[i-1][j], matrix[i][j]):
            print(f"problem 3 row={i} col={j}")