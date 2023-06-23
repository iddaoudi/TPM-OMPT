static void get_ompt_callback_task_create(
    ompt_data_t *encountering_task_data,
    const ompt_frame_t
        *encountering_task_frame,
    ompt_data_t *new_task_data,
    int flags, int has_dependences,
    const void *codeptr_ra)
{
    // Capture the tasks created
    if (flags & ompt_task_explicit) // Only capture explicit tasks
    {
        // Necessary to store the pointer, will be catched in the schedule callback
        new_task_data->value = (uint64_t)codeptr_ra;

        Task *current_task = algorithm.head;
        // Check if task has already been registered
        while (current_task != NULL)
        {
            if (current_task->identifier == (uint64_t)codeptr_ra)
            {
                return;
            }
            current_task = current_task->next;
        }
        // Register a new task type
        Task *task = malloc(sizeof(Task));
        task->identifier = (uint64_t)codeptr_ra;
        sprintf(task->name, "task%d", ++algorithm.count);
        task->next = NULL;
        for (int i = 0; i < NEVENTS; i++)
        {
            task->counters[i] = 0;
        }
        if (algorithm.head == NULL)
        {
            algorithm.head = task;
            algorithm.tail = task;
        }
        else
        {
            algorithm.tail->next = task;
            algorithm.tail = task;
        }
    }
}

static void get_ompt_callback_task_schedule(ompt_data_t *prior_task_data,
                                            ompt_task_status_t prior_task_status,
                                            ompt_data_t *next_task_data)
{
    int flag = 0;
    // Task starts running
    if (next_task_data->value) // Only capture explicit tasks
    {
        flag++;

        // Start monitoring
        pthread_mutex_lock(&mutex);
        if (TPM_POWER)
        {
            unsigned int cpu = sched_getcpu();
            Task *task = TPM_find_task((uint64_t)next_task_data->value);
            if (task != NULL)
            {
                char *signal_control_task_on_cpu = TPM_str_and_int_to_str(task->name, cpu);
                TPM_zmq_send_signal(zmq_request, signal_control_task_on_cpu);
                free(signal_control_task_on_cpu);
            }
            else
            {
                printf("Task was never found\n");
            }
        }
        if (TPM_PAPI)
        {
            /* Start PAPI counters */
            memset(values, 0, sizeof(values));
            int ret = PAPI_start(eventset);
            if (ret != PAPI_OK)
            {
                fprintf(stderr, "PAPI_start error: %s\n", PAPI_strerror(ret));
                exit(EXIT_FAILURE);
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    // Task finishes
    if (prior_task_status == ompt_task_complete)
    {
        flag++;

        pthread_mutex_lock(&mutex);
        if (TPM_PAPI)
        {
            // Stop monitoring
            int ret = PAPI_stop(eventset, values);
            if (ret != PAPI_OK)
            {
                fprintf(stderr, "PAPI_stop error: %s\n", PAPI_strerror(ret));
                exit(EXIT_FAILURE);
            }
            Task *task = TPM_find_task((uint64_t)prior_task_data->value);
            if (task != NULL)
            {
                // collect PAPI counters and add them to the task's counters
                for (int i = 0; i < NEVENTS; i++)
                {
                    task->counters[i] += values[i];
                }
                task->counters[NEVENTS]++;
            }
        }
        pthread_mutex_unlock(&mutex);
    }

    if (flag > 2)
    {
        printf("Task scheduled and finished at the same callback. Doesn't make sense!\n");
    }
}

static void get_ompt_callback_parallel_begin(
    ompt_data_t *encountering_task_data,
    const ompt_frame_t *encountering_task_frame,
    ompt_data_t *parallel_data,
    unsigned int requested_parallelism,
    int flags,
    const void *codeptr_ra)
{
    if (start_time == 0.0)
    {
        start_time = omp_get_wtime();
    }
}

static void get_ompt_callback_parallel_end(
    ompt_data_t *parallel_data,
    ompt_data_t *encountering_task_data,
    int flags,
    const void *codeptr_ra)
{
    end_time = omp_get_wtime();
}