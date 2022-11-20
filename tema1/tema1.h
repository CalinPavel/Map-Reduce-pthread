#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <algorithm>

pthread_barrier_t barrier;

struct mapper_memory
{
    std::vector<std::string> *files;
    std::vector<std::vector<int>> mem;
    std::vector<std::vector<int>> *compare_memory;
    int M;
    int R;
    int size;
    int reducer_id;
};

void get_parameters(int argc, char **argv, int *M, int *R);
void *mapper_function(void *arg);
void *reducer(void *arg);
