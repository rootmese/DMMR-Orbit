#include "scheduler_preemptive.h"
#include <sys/time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

static size_t scheduler_connection_size = 0;
static size_t scheduler_connection_count = 0;
static struct scheduler_connection *slots; // neste buffer são armazedos os pools preemptivos
static pthread_mutex_t slots_mutex = PTHREAD_MUTEX_INITIALIZER;
                                                  

// --- Estrutura de tempo ---
static uint64_t _get_time_us() {
    struct timeval tv;
    gettimeofday(&tv, 0);
    return (tv.tv_sec * 1000000ULL) + tv.tv_usec;
}

static uint64_t _get_monotonic_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((uint64_t)ts.tv_sec * 1000000ULL) + (ts.tv_nsec / 1000);
}

// TODO, outra thread ficará constantemente procurando novas conexões para inserir em slots e realocar caso seja necessário, eu queria evitar mutex, porém creio que compensa
static int ensure_capacity(void) {
    if (scheduler_connection_size >= scheduler_connection_current_size)
    {
        pthread_mutex_lock(&slots_mutex);
        scheduler_connection_size *= 2;
        struct scheduler_connection *new_slots = realloc(slots, new_capacity * sizeof(struct scheduler_connection));
        if (!new_slots)
        {
            pthread_mutex_unlock(&slots_mutex);
            return EOF;
        }
        slots = new_slots;
        pthread_mutex_unlock(&slots_mutex);
        return 0;
    }
    else
        return EOF;
}

static void* _collector(void* arg) {
    scheduler_preemptive* s = (scheduler_preemptive*)arg;

    do {
        // Aqui você pode iterar sobre as sessões do session manager
        // e adicionar novas conexões ao buffer 'slots'.

        // Exemplo de estrutura esperada:
        // struct session_connection* new_conn = session_manager_get_new();
        // if (new_conn) { inserir no slots... }

        usleep(1000); // Evita uso excessivo de CPU
    }while (~0);

    return 0;
}


static void* _reorder(void* arg) {
    scheduler_preemptive* s = (scheduler_preemptive*)arg;
    do {
        pthread_mutex_lock(&slots_mutex);
        for (int i = 1; i < scheduler_connection_count; i++) {
            struct scheduler_connection key = *(slots + i);
            uint64_t key_deadline = key.realtime_deadline_us;
            int left = 0;
            int right = i - 1;
            while (left <= right) {
                int mid = (left + right) / 2;
                if ((*(slots + mid)).realtime_deadline_us <= key_deadline)
                    left = mid + 1;
                else
                    right = mid - 1;
            }
            for (int j = i - 1; j >= left; j--) {
                *(slots + j + 1) = *(slots + j);
            }
            *(slots + left) = key;
        }
        // TODO validar estender o lock para a rotina abaixo
        uint64_t now = _get_monotonic_time_us();
        for (int i = 0; i < scheduler_connection_count; i++) {
            struct scheduler_connection* conn = slots + i;
            if (now + SCHEDULER_DEADLINE_TOLERANCE_US >= conn->realtime_deadline_us) {
                _trigger_send(conn);
            } else {
                break;
            }
        }
        pthread_mutex_unlock(&slots_mutex);
        usleep(100);
    } while(~0);
}

static void _trigger_send(scheduler_connection* conn) {
    printf("[SEND] Sessão em deadline (%llu us), ponteiro: %p\n",
           (unsigned long long)conn->realtime_deadline_us,
           conn->session_ptr);
    conn->active = 0;

    // Aqui você pode usar o ponteiro para a sessão, ex:
    // session_send((session_connection*)conn->session_ptr);
}

static void reload(struct cfg_server_server* __cfg_server){
    p->deadline = __cfg_server->scheduler_preemptive_deadline;
}

struct dmmr_scheduler* new_dmmr_scheduler(struct cfg_server_server *__cfg_server)
{
    struct dmmr_scheduler *p = (struct dmmr_scheduler*)calloc(1, sizeof(struct dmmr_scheduler));
    if(p)
    {
        slots = (struct struct scheduler_connection*)calloc(0x400, sizeof(struct scheduler_connection));
        if(!slots)
            goto error;
        scheduler_connection_size = 0x400;
        p->deadline = __cfg_server->scheduler_preemptive_deadline;
        p->reload = reload;
        pthread_mutex_init(&slots_mutex, 0);
        pthread_create(&p->worker_thread, 0, _reorder, p);
        pthread_create(&p->collector_thread, 0, _collector, p);
        return p;
    }
    error:
    if(p)
        free(p);
    return 0;
}
