#define MAX_EVENTS 4

int events[MAX_EVENTS];
char *events_strings[MAX_EVENTS];
int NEVENTS;

int eventset;
long long values[MAX_EVENTS];

typedef struct
{
    long long values[MAX_EVENTS + 1];
} CounterData;

typedef struct Task
{
    uint64_t identifier;
    char name[20];
    long long counters[MAX_EVENTS];
    struct Task *next;
} Task;

typedef struct Algorithm
{
    Task *head;
    Task *tail;
    int count;
} Algorithm;

Algorithm algorithm = {NULL, NULL, 0};

Task *TPM_find_task(uint64_t identifier)
{
    Task *current = algorithm.head;
    while (current != NULL)
    {
        if (current->identifier == identifier)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

char *TPM_str_and_int_to_str(const char *task_name, int cpu)
{
    if (task_name == NULL)
    {
        fprintf(stderr, "Error: null pointer passed to function\n");
        exit(EXIT_FAILURE);
    }

    // Allocate enough space for task, integer, space and null terminator
    // The number 6 is for the maximum digits of 9999 (4 digits)
    size_t new_string_size = TPM_STRING_SIZE + 6;
    char *new_task_name = (char *)malloc(new_string_size * sizeof(char));

    if (new_task_name == NULL)
    {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    snprintf(new_task_name, new_string_size, "%s %d", task_name, cpu);

    return new_task_name;
}

char *TPM_str_and_double_to_str(const char *string, double value)
{
    if (string == NULL)
    {
        fprintf(stderr, "Error: null pointer passed to function\n");
        exit(EXIT_FAILURE);
    }

    // Allocate enough space for task, double, space and null terminator
    // The number 15 is for the maximum digits of a double (scientific notation)
    size_t new_string_size = TPM_STRING_SIZE + 15;
    char *new_string = (char *)malloc(new_string_size * sizeof(char));

    if (new_string == NULL)
    {
        fprintf(stderr, "Error: memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    snprintf(new_string, new_string_size, "%s %.4f", string, value);

    return new_string;
}