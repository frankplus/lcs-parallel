release: main.c
	mpCC main.c -o lcs

debug: main.c
	mpicc main.c -o lcs
	mpirun -np 4 ./lcs input1000.txt