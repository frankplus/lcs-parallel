release: main.c
	mpCC main.c -o lcs -O3

debug: main.c
	mpicc main.c -o lcs
	mpirun -np 4 ./lcs input10000.txt