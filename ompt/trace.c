#include "trace.h"

int ompt_initialize(ompt_function_lookup_t lookup, int initial_device_num,
                    ompt_data_t *tool_data)
{
    // Register callbacks
    ompt_set_callback_t ompt_set_callback = (ompt_set_callback_t)lookup("ompt_set_callback");
    register_callback(ompt_callback_task_create);
    register_callback(ompt_callback_task_schedule);
    register_callback(ompt_callback_parallel_begin);
    register_callback(ompt_callback_parallel_end);

    // Get environment variables
    TPM_ALGORITHM = getenv("TPM_ALGORITHM");
    TPM_PAPI = atoi(getenv("TPM_PAPI_SET"));
    TPM_POWER = atoi(getenv("TPM_POWER_SET"));
    TPM_TASK_TIME = atoi(getenv("TPM_TASK_TIME"));
    TPM_TASK_TIME_TASK = getenv("TPM_TASK_TIME_TASK");

    // Initialize ZMQ
    if (TPM_POWER)
    {
        zmq_context = zmq_ctx_new();
        zmq_request = zmq_socket(zmq_context, ZMQ_PUSH);

        TPM_zmq_connect_client(zmq_request);
        TPM_zmq_send_signal(zmq_request, "energy 0");
    }

    // Initialize PAPI
    if (TPM_PAPI)
    {
        int papi_version = PAPI_library_init(PAPI_VER_CURRENT);
        if (papi_version != PAPI_VER_CURRENT && papi_version > 0)
        {
            fprintf(stderr, "PAPI library version mismatch: %s\n", PAPI_strerror(papi_version));
            exit(EXIT_FAILURE);
        }
        else if (papi_version < 0)
        {
            fprintf(stderr, "PAPI library init error: %s\n", PAPI_strerror(papi_version));
            exit(EXIT_FAILURE);
        }
        /* Threaded PAPI initialization */
        int ret;
        if ((ret = PAPI_thread_init(pthread_self)) != PAPI_OK)
        {
            fprintf(stderr, "PAPI_thread_init error: %s\n", PAPI_strerror(ret));
            exit(EXIT_FAILURE);
        }

        /* Add PAPI events */
        // eventset = PAPI_NULL;
        // ret = PAPI_create_eventset(&eventset);
        // if (ret != PAPI_OK)
        // {
        //     fprintf(stderr, "PAPI_create_eventset error: %s\n", PAPI_strerror(ret));
        //     exit(EXIT_FAILURE);
        // }
        TPM_PAPI_COUNTERS = atoi(getenv("TPM_PAPI_COUNTERS"));
        if (TPM_PAPI_COUNTERS == 1)
        {
            events[0] = PAPI_L3_TCM;
            events[1] = PAPI_TOT_INS;
            events[2] = PAPI_TOT_CYC;
            events[3] = PAPI_RES_STL;
            events_strings[0] = "PAPI_L3_TCM";
            events_strings[1] = "PAPI_TOT_INS";
            events_strings[2] = "PAPI_TOT_CYC";
            events_strings[3] = "PAPI_RES_STL";
            NEVENTS = 4;
        }
        else if (TPM_PAPI_COUNTERS == 2)
        {
            events[0] = PAPI_L2_TCR;
            events[1] = PAPI_L2_TCW;
            events_strings[0] = "PAPI_L2_TCR";
            events_strings[1] = "PAPI_L2_TCW";
            NEVENTS = 2;
        }
        else if (TPM_PAPI_COUNTERS == 3)
        {
            events[0] = PAPI_L3_TCR;
            events[1] = PAPI_L3_TCW;
            events_strings[0] = "PAPI_L3_TCR";
            events_strings[1] = "PAPI_L3_TCW";
            NEVENTS = 2;
        }
        else if (TPM_PAPI_COUNTERS == 4)
        {
            events[0] = PAPI_VEC_DP;
            events_strings[0] = "PAPI_VEC_DP";
            NEVENTS = 1;
        }
        // ret = PAPI_add_events(eventset, events, NEVENTS);
        // if (ret != PAPI_OK)
        // {
        //     fprintf(stderr, "PAPI_add_events error: %s\n", PAPI_strerror(ret));
        //     exit(EXIT_FAILURE);
        // }
    }
    return 1;
}

void ompt_finalize(ompt_data_t *tool_data)
{
    free(tool_data->ptr);
    if (TPM_POWER)
    {
        TPM_zmq_send_signal(zmq_request, "energy 1");
        char *signal_execution_time = TPM_str_and_double_to_str("time", end_time - start_time);
        TPM_zmq_send_signal(zmq_request, signal_execution_time);
        free(signal_execution_time);
        TPM_zmq_close(zmq_request, zmq_context);
    }
    if (TPM_PAPI)
    {
#pragma omp taskwait

        /* Get L3 cache size */
        long l3_cache_size;
#ifdef _SC_LEVEL3_CACHE_SIZE
        l3_cache_size = sysconf(_SC_LEVEL3_CACHE_SIZE);
#else
        fprintf(stderr, "_SC_LEVEL3_CACHE_SIZE is not available\n");
        exit(EXIT_FAILURE);
#endif
        if (l3_cache_size == -1)
        {
            fprintf(stderr, "L3 cache size sysconf failed\n");
            exit(EXIT_FAILURE);
        }
        dump(l3_cache_size);
    }

    Task *current = algorithm.head;
    while (current != NULL)
    {
        Task *next = current->next;
        PAPI_destroy_eventset(&current->eventset);
        free(current);
        current = next;
    }
    PAPI_shutdown();
}

ompt_start_tool_result_t *ompt_start_tool(unsigned int omp_version,
                                          const char *runtime_version)
{
    static ompt_start_tool_result_t ompt_start_tool_result = {&ompt_initialize, &ompt_finalize, {.value = 0}};
    return &ompt_start_tool_result;
}