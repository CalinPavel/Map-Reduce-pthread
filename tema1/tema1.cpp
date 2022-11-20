#include "tema1.h"

using namespace std;

pthread_mutex_t lock;

void get_parameters(int argc, char **argv, int *M, int *R, vector<string> *files)
{
	if (argc < 4)
	{
		printf("Numar insuficient de parametri!\n");
		exit(1);
	}

	*M = atoi(argv[1]);
	*R = atoi(argv[2]);

	string text;
	ifstream MyReadFile(argv[3]);
	getline(MyReadFile, text);
	while (getline(MyReadFile, text))
	{
		(*files).push_back(text);
	}
	MyReadFile.close();
}

int binarySearch(vector<int> arr, int l, int r, int x)
{
	while (l <= r)
	{
		int m = l + (r - l) / 2;

		if (arr[m] == x)
			return m;

		if (arr[m] < x)
			l = m + 1;

		else
			r = m - 1;
	}
	return -1;
}

void *mapper_function(void *parameters)
{
	struct mapper_memory *mapper = (struct mapper_memory *)parameters;

	int i;
next_file:
	// extragere fisier din lista
	pthread_mutex_lock(&lock);
	std::vector<string> *file = mapper->files;
	string file_to_check;
	if (!(*file).empty())
	{
		file_to_check = (*file).back();
		(*file).pop_back();
	}
	pthread_mutex_unlock(&lock);

	string text;
	ifstream MyReadFile(file_to_check);

	std::vector<std::vector<int>> *check_memory = mapper->compare_memory;
	std::vector<std::vector<int>> mem = mapper->mem;

	vector<int> store;
	store.push_back(-1);
	for (i = 0; i <= mapper->R + 2; i++)
	{
		mapper->mem.push_back(store);
	}
	getline(MyReadFile, text);
	while (getline(MyReadFile, text))
	{
		for (i = 2; i <= mapper->R + 1; i++)
		{
			// se cauta in memoria precalculata
			if (binarySearch((*check_memory)[i], 0, (*check_memory)[i].size() - 1, stoi(text)) != -1)
			{
				mapper->mem[i].push_back(stoi(text));
			}
		}
	}
	MyReadFile.close();

	// se proceseaza urmatorul fisier din vector
	if (!(*file).empty())
	{
		goto next_file;
	}

	pthread_barrier_wait(&barrier);
	pthread_exit(NULL);
}

void *reducer_function(void *parameters)
{
	// se verifica finalitatea thread-urilor de tip mapper
	pthread_barrier_wait(&barrier);
	pthread_mutex_lock(&lock);

	int id, size;
	vector<mapper_memory> *reducer_mem = (vector<mapper_memory> *)parameters;
	mapper_memory reducer = (*reducer_mem)[0];
	// asociere exponent 
	id = reducer.reducer_id;
	(*reducer_mem)[0].reducer_id++;
	pthread_mutex_unlock(&lock);

	size = reducer.size - 1;

	vector<int> data;

	for (int i = 0; i <= size; i++)
	{
		mapper_memory reducer1 = (*reducer_mem)[i]; 
		// parcurgerea vectorilor corespunzatori exponentului cautat
		for (auto j = reducer1.mem[id].begin() + 1; j != reducer1.mem[id].end(); ++j)
		{
			// verificare unicitate numar
			if (std::find(data.begin(), data.end(), *j) == data.end())
			{
				data.push_back(*j);
			}
		}
	}
	// scriere in fisier
	string file = "out";
	string to_add = std::to_string(id);
	string fullname = file.append(to_add).append(".txt");

	ofstream MyFile(fullname);
	MyFile << data.size();
	MyFile.close();

	pthread_exit(NULL);
}

int main(int argc, char **argv)
{
	int M, R, num_threads, r, i;
	vector<string> files;
	void *status;

	get_parameters(argc, argv, &M, &R, &files);
	num_threads = M + R;

	pthread_t threads[num_threads];
	pthread_barrier_init(&barrier, NULL, M + R);

	// precalculare puteri perfecte

	std::vector<std::vector<int>> compare_memory;
	int exponent;
	vector<int> store;

	store.push_back(-1);
	compare_memory.push_back(store);
	compare_memory.push_back(store);
	store.clear();
	unsigned long long value, count = 1;
	for (exponent = 2; exponent <= R + 1; exponent++)
	{
	next:
		value = count;
		for (i = 1; i <= exponent - 1; i++)
		{
			value *= count;
		}

		if (value < INT32_MAX)
		{
			store.push_back((int)value);
			value = count++;
			goto next;
		}
		compare_memory.push_back(store);
		count = 1;
		store.clear();
	}

	vector<mapper_memory> mapper_mem;
	for (i = 0; i < M; i++)
	{
		mapper_mem.push_back(*(new mapper_memory()));
		mapper_mem[i].files = &files;
		mapper_mem[i].M = M;
		mapper_mem[i].R = R;
		mapper_mem[i].compare_memory = &compare_memory;
	}
	// primul exponent verificat
	mapper_mem[0].reducer_id = 2;
	// marimea vectorului de structuri
	mapper_mem[0].size = mapper_mem.size();

	for (int i = 0; i < M + R; i++)
	{
		if (i < M)
		{
			r = pthread_create(&threads[i], NULL, mapper_function, (void *)&mapper_mem[i]);
			if (r)
			{
				printf("Eroare la crearea thread-ului %d\n", i);
				exit(-1);
			}
		}
		else
		{
			r = pthread_create(&threads[i], NULL, reducer_function, (void *)&mapper_mem);
			if (r)
			{
				printf("Eroare la crearea thread-ului %d\n", i);
				exit(-1);
			}
		}
	}

	for (i = 0; i < M + R; i++)
	{
		r = pthread_join(threads[i], &status);

		if (r)
		{
			printf("Eroare la asteptarea thread-ului %d\n", i);
			exit(-1);
		}
	}

	pthread_barrier_destroy(&barrier);
	return 0;
}
