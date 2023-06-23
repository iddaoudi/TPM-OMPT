char *ALGORITHM;
int NTHREADS;
int MATRIX;
int TILE;

int frequency_to_set;
int default_frequency;
int combination_of_tasks;

// FIXME assuming we will always get exactly 4 different tasks
static const char *tpm_tasks[] = {"task1", "task2", "task3", "task4"};

cvector_vector_type(char *) tasks_vec;
