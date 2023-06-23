#include <pthread.h>

typedef struct
{
    const char *algorithm;
    const char **task_names;
    int num_tasks;
} AlgorithmTasks;

void TPM_power_control(int selected_case, const char *task, unsigned int cpu,
                       unsigned long frequency_to_set,
                       unsigned long original_frequency)
{
    int num_tasks = sizeof(tpm_tasks) / sizeof(tpm_tasks[0]);
    if (selected_case >= 1 && selected_case <= ((1 << num_tasks) - 1))
    {
        int task_mask = selected_case - 1;
        int task_found = 0;
        for (int i = 0; i < num_tasks; ++i)
        {
            if (!strcmp(task, tpm_tasks[i]) && (task_mask & (1 << i)))
            {
                TPM_power_set_frequency(cpu, frequency_to_set);
                task_found = 1;
                break;
            }
        }
        if (!task_found)
        {
            TPM_power_set_frequency(cpu, original_frequency);
        }
    }
    else if (selected_case == (1 << num_tasks))
    {
        TPM_power_set_frequency(cpu, frequency_to_set);
    }
}
