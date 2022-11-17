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
	if (r >= l)
	{
		int mid = l + (r - l) / 2;
		if (arr[mid] == x)
			return mid;
		if (arr[mid] > x)
			return binarySearch(arr, l, mid - 1, x);
		return binarySearch(arr, mid + 1, r, x);
	}
	return -1;
}

void *mapper_function(void *parameters)
{
	struct mapper_memory *mapper = (struct mapper_memory *)parameters;

	int i;
	pthread_mutex_lock(&lock);
	std::vector<string> *file = mapper->files;
	string file_to_check;
	if (!(*file).empty())
	{
		file_to_check = (*file).back();
		(*file).pop_back();
	}

	string text;
	ifstream MyReadFile(file_to_check);
	// getline(MyReadFile, text);

	std::vector<std::vector<int>> *check_memory = mapper->compare_memory;
	std::vector<std::vector<int>> mem = mapper->mem;

	vector<int> store;
	store.push_back(-1);
	for (i = 0; i <= mapper->R + 1; i++)
	{
		mapper->mem.push_back(store);
	}

	while (getline(MyReadFile, text))
	{
		for (i = 2; i <= mapper->R; i++)
		{
			if (binarySearch((*check_memory)[i], 0, (*check_memory)[i].size() - 1, stoi(text)) != -1)
			{
				// store.push_back(stoi(text));
				mapper->mem[i].push_back(stoi(text));
			}
		}
		// cout << stoi(text) << "\n";
	}

	for (int i = 0; i < mapper->mem.size(); i++)
	{
		for (int j = 0; j < mapper->mem[i].size(); j++)
		{
			cout << mapper->mem[i][j] << " ";
		}
		cout << endl;
	}

	cout << "///////////////////\n";
	pthread_mutex_unlock(&lock);

	// cout << (*check_memory)[1][1] << "\n";

	// for(auto i = (*check_memory)[3].begin() ; i !=(*check_memory)[3].end() ; ++i){
	// 	cout << *i << "\n";
	// }
	// cout << (*check_memory)[3].size()-1;

	MyReadFile.close();
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
	int arguments[num_threads];

	pthread_barrier_init(&barrier, NULL, num_threads);

	///////////////////////////////////////////////////// build exp
	std::vector<std::vector<int>> compare_memory;
	int exponent, line = 0;
	vector<int> store;

	store.push_back(-1);
	// line 0
	compare_memory.push_back(store);
	// line 1
	compare_memory.push_back(store);
	store.clear();
	unsigned int value, count = 1;
	for (exponent = 2; exponent <= R + 1; exponent++)
	{
	next:
		value = count;
		// cout << "Value:" << value << "\n";
		for (i = 1; i <= exponent - 1; i++)
		{
			value *= count;
		}
		// cout << value << "\n";

		if (value < 100)
		{
			store.push_back((int)value);
			value = count++;
			// cout << "Count" << count << "\n";
			goto next;
		}
		// cout << "/////////////////\n";
		compare_memory.push_back(store);
		count = 1;
		store.clear();
	}
	/////////////////////////////////////////////////////build exp

	vector<mapper_memory> mapper_mem;
	for (i = 0; i < M; i++)
	{
		mapper_mem.push_back(mapper_memory());
		mapper_mem[i].files = &files;
		mapper_mem[i].M = M;
		mapper_mem[i].R = R;
		mapper_mem[i].compare_memory = &compare_memory;
	}

	// cout << files[1] << "\n";
	// cout << mapper_mem[0].files;

	for (int i = 0; i < M; i++)
	{
		r = pthread_create(&threads[i], NULL, mapper_function, (void *)&mapper_mem[i]);
		if (r)
		{
			printf("Eroare la crearea thread-ului %d\n", i);
			exit(-1);
		}
	}

	// for (i = 0; i < R; i++)
	// {
	// 	arguments[i] = i;
	// 	r = pthread_create(&threads[i], NULL, reducer, &arguments[i]);

	// 	if (r)
	// 	{
	// 		printf("Eroare la crearea thread-ului %d\n", i);
	// 		exit(-1);
	// 	}
	// }

	for (i = 0; i < M; i++)
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
