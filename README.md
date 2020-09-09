# lcs-parallel
This is a parallel implementation of the longest common subsequence algorithm in MPI.\
In this repository you can find the following content:
- some randomly generated inputs of different size.
- `generate_input.py` is the python script used to generate the inputs.
- `main.c` contains the implementation of the parallel and also the sequential LCS algorithm.
- `Makefile`: run `make` to compile the code with mpCC
- A `lcs_parallel.job` file to run the program on a load leveler.

## Report
![report page 1](report_jpg/report_parallel_LCS-page-001.jpg "report page 1")
![report page 2](report_jpg/report_parallel_LCS-page-002.jpg "report page 2")
![report page 3](report_jpg/report_parallel_LCS-page-003.jpg "report page 3")

## Author
Francesco Pham