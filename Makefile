debug: main.c
	mpicc main.c -o lcs
	mpirun -np 4 ./lcs