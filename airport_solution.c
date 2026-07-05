#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#include <stdarg.h>

// ----------- CONSTANTS -----------
#define MAX_PLANES 10
#define TAXIWAY_SIZE 3
#define NUM_GATES 3

// ----------- COLORS -----------
#define RESET   "\033[0m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define MAGENTA "\033[1;35m"
#define CYAN    "\033[1;36m"

// ----------- SEMAPHORES -----------
// binary semaphore (only 1 plane can land or takeoff)
sem_t runway;

// counting semaphore (max 3 planes in taxiway)
sem_t taxiway_slots;

// signal plane → tower
sem_t request_landing;

// tower allows specific plane i to land
sem_t permit_landing[MAX_PLANES + 1];

// plane entered taxiway, notify operator
sem_t plane_in_queue;

// operator assigns gate to plane i
sem_t gate_assigned[MAX_PLANES + 1];

// availability of gate A,B,C
sem_t gates[NUM_GATES];

// ----------- MUTEXES -----------
// protects taxiway queue
pthread_mutex_t queue_mutex;

// protects landing request queue
pthread_mutex_t request_mutex;

// prevent mixed output
pthread_mutex_t print_mutex;

// ----------- SHARED DATA -----------
// taxiway queue
int taxiway_queue[TAXIWAY_SIZE];
int front = 0, rear = 0;

// landing request queue
int landing_queue[MAX_PLANES + 1];
int l_front = 0, l_rear = 0;

// assigned gates
int assigned_gate[MAX_PLANES + 1];

// ----------- SAFE PRINT -----------
void safe_print(const char* color, const char* format, ...)
{
    va_list args;
    va_start(args, format);

    pthread_mutex_lock(&print_mutex);
    printf("%s", color);
    vprintf(format, args);
    printf("%s", RESET);
    fflush(stdout);
    pthread_mutex_unlock(&print_mutex);

    va_end(args);
}

// ----------- QUEUE FUNCTIONS -----------
void enqueue_taxiway(int id) {
    taxiway_queue[rear] = id;
    rear = (rear + 1) % TAXIWAY_SIZE;
}

int dequeue_taxiway() {
    int id = taxiway_queue[front];
    front = (front + 1) % TAXIWAY_SIZE;
    return id;
}

void enqueue_landing(int id) {
    landing_queue[l_rear] = id;
    l_rear = (l_rear + 1) % (MAX_PLANES + 1);
}

int dequeue_landing() {
    int id = landing_queue[l_front];
    l_front = (l_front + 1) % (MAX_PLANES + 1);
    return id;
}

// ----------- TOWER THREAD -----------
void* tower(void* arg)
{
    int total = *(int*)arg;
    safe_print(CYAN, "Tower: Ready\n");

    for(int i = 0; i < total; i++)
    {
        sem_wait(&request_landing);

        pthread_mutex_lock(&request_mutex);
        int plane_id = dequeue_landing();
        pthread_mutex_unlock(&request_mutex);

        sem_wait(&taxiway_slots);
        sem_wait(&runway);

        safe_print(CYAN, "Tower: Plane-%d clear to land\n", plane_id);

        sem_post(&permit_landing[plane_id]);
    }

    safe_print(CYAN, "Tower: Exit\n");
    pthread_exit(NULL);
}

// ----------- OPERATOR THREAD -----------
void* operator(void* arg)
{
    int total = *(int*)arg;
    int gate_idx = 0;
    safe_print(BLUE, "Operator: Ready\n");

    for(int i = 0; i < total; i++)
    {
        sem_wait(&plane_in_queue);

        pthread_mutex_lock(&queue_mutex);
        int plane_id = dequeue_taxiway();
        pthread_mutex_unlock(&queue_mutex);

        int gate = gate_idx;
        gate_idx = (gate_idx + 1) % NUM_GATES;

        sem_wait(&gates[gate]);
        assigned_gate[plane_id] = gate;

        safe_print(BLUE, "Operator: Plane-%d assigned Gate %c\n", plane_id, 'A' + gate);

        sem_post(&gate_assigned[plane_id]);
    }

    safe_print(BLUE, "Operator: Exit\n");
    pthread_exit(NULL);
}

// ----------- PLANE THREAD -----------
void* plane(void* arg)
{
    int id = *(int*)arg;

    safe_print(YELLOW, "Plane-%d: Approaching airport\n", id);

    pthread_mutex_lock(&request_mutex);
    enqueue_landing(id);
    pthread_mutex_unlock(&request_mutex);
    sem_post(&request_landing);

    sem_wait(&permit_landing[id]);

    safe_print(GREEN, "Plane-%d: Landing\n", id);
    sleep(1);

    sem_post(&runway);

    pthread_mutex_lock(&queue_mutex);
    enqueue_taxiway(id);
    pthread_mutex_unlock(&queue_mutex);

    safe_print(GREEN, "Plane-%d: Entered taxiway queue\n", id);
    sem_post(&plane_in_queue);

    sem_wait(&gate_assigned[id]);
    int gate = assigned_gate[id];

    safe_print(GREEN, "Plane-%d: Docked at Gate %c\n", id, 'A' + gate);

    sem_post(&taxiway_slots);

    sleep(rand() % 2 + 4);

    safe_print(YELLOW, "Plane-%d: Ready for takeoff\n", id);

    sem_wait(&runway);
    safe_print(MAGENTA, "Plane-%d: Taking off\n", id);
    sleep(1);

    sem_post(&runway);
    sem_post(&gates[gate]);

    safe_print(MAGENTA, "Plane-%d: Departed\n", id);

    pthread_exit(NULL);
}

// ----------- MAIN -----------
int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        printf("Usage: %s <number_of_planes>\n", argv[0]);
        return 1;
    }

    int total_planes = atoi(argv[1]);
    if(total_planes < 4 || total_planes > 10)
    {
        printf("Number of planes must be between 4 and 10\n");
        return 1;
    }

    srand(time(NULL));

    safe_print(CYAN, "Airport Simulation Started with %d planes\n\n", total_planes);

    if(sem_init(&runway,0,1)!=0){ perror("runway"); exit(1); }
    if(sem_init(&taxiway_slots,0,TAXIWAY_SIZE)!=0){ perror("taxiway"); exit(1); }
    if(sem_init(&request_landing,0,0)!=0){ perror("request"); exit(1); }
    if(sem_init(&plane_in_queue,0,0)!=0){ perror("queue"); exit(1); }

    for(int i=0;i<NUM_GATES;i++)
        if(sem_init(&gates[i],0,1)!=0){ perror("gate"); exit(1); }

    for(int i=0;i<MAX_PLANES+1;i++){
        if(sem_init(&permit_landing[i],0,0)!=0){ perror("permit"); exit(1); }
        if(sem_init(&gate_assigned[i],0,0)!=0){ perror("gate_assign"); exit(1); }
    }

    pthread_mutex_init(&queue_mutex,NULL);
    pthread_mutex_init(&request_mutex,NULL);
    pthread_mutex_init(&print_mutex,NULL);

    pthread_t tower_thread;
    pthread_t operator_thread;
    pthread_t planes[MAX_PLANES];
    int ids[MAX_PLANES];

    if(pthread_create(&tower_thread,NULL,tower,&total_planes)!=0){ perror("tower"); exit(1); }
    if(pthread_create(&operator_thread,NULL,operator,&total_planes)!=0){ perror("operator"); exit(1); }

    for(int i=0;i<total_planes;i++){
        ids[i]=i+1;
        if(pthread_create(&planes[i],NULL,plane,&ids[i])!=0){ perror("plane"); exit(1); }
        sleep(rand()%3 + 2);
    }

    for(int i=0;i<total_planes;i++) pthread_join(planes[i],NULL);
    pthread_join(tower_thread,NULL);
    pthread_join(operator_thread,NULL);

    safe_print(CYAN,"\nAll threads finished\nProgram terminated\n");

    sem_destroy(&runway);
    sem_destroy(&taxiway_slots);
    sem_destroy(&request_landing);
    sem_destroy(&plane_in_queue);

    for(int i=0;i<NUM_GATES;i++) sem_destroy(&gates[i]);
    for(int i=0;i<MAX_PLANES+1;i++){
        sem_destroy(&permit_landing[i]);
        sem_destroy(&gate_assigned[i]);
    }

    pthread_mutex_destroy(&queue_mutex);
    pthread_mutex_destroy(&request_mutex);
    pthread_mutex_destroy(&print_mutex);

    return 0;
}
