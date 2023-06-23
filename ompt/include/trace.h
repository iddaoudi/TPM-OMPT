#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include <sched.h>
#include <stddef.h>
#include <omp.h>
#include <ompt.h>

#include "cvector.h"
#include "pthread.h"
#include "zmq.h"
#include "papi.h"

#define TPM_MESSAGE_SIZE 26
#define TPM_STRING_SIZE 10
#define TPM_FILENAME_SIZE 64

pthread_mutex_t mutex;

int TPM_PAPI = 0;
int TPM_POWER = 0;
int TPM_TASK_TIME = 0;
int TPM_PAPI_COUNTERS = 0;

char *TPM_ALGORITHM = NULL;
char *TPM_TASK_TIME_TASK = NULL;

static double start_time = 0.0;
static double end_time = 0.0;

#include "zutils.h"
#include "client.h"
#include "utils.h"
#include "callbacks.h"
#include "dump.h"

#define register_callback_t(name, type)                                           \
    do                                                                            \
    {                                                                             \
        type f_##name = &get_##name;                                              \
        if (ompt_set_callback(name, (ompt_callback_t)f_##name) == ompt_set_never) \
            printf("0: Could not register callback '" #name "'\n");               \
    } while (0)

#define register_callback(name) register_callback_t(name, name##_t)

cvector_vector_type(uint64_t) task_identifiers;
