from math import ceil
ROW = 15
COL = 15
SIZE = 4

# Utility function to print a matrix 
def printMatrix(matrix) : 
    for i in range(0, ROW) : 
        for j in range(0, COL) : 
            print(matrix[i][j], end = " ") 
              
        print() 
  
matrix = [ [ -1 for i in range(ROW) ] for j in range(COL) ] 

for rank in range(0,SIZE):
    for line in range(1, (ROW + COL)) : 
        start_col = max(0, line - ROW) 
        count = min(line, (COL - start_col), ROW) 

        if count < SIZE:
            start = rank
            end = min(rank+1, count)
        else:
            block_len = count / SIZE
            start = round(block_len*rank)
            end = round(block_len*(rank+1))

        for j in range(start, end):
            row = min(ROW, line) - j - 1
            col = start_col + j
            if matrix[row][col] != -1:
                print(f"error already calculated row={row} col={col}")
            matrix[row][col] = rank

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