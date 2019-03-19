#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>
#include "mythread.h"
#include "interrupt.h"
#include "queue.h"

TCB* scheduler();
void activator();
void timer_interrupt(int sig);
void disk_interrupt(int sig);

/* Array of state thread control blocks: the process allows a maximum of N threads */
static TCB t_state[N];

/* Current running thread */
static TCB* running;

/* Variable indicating if the library is initialized (init == 1) or not (init == 0) */
static int init=0;

/* Thread control block for the idle thread */
static TCB idle;
static void idle_function(){
	while(1);
}

//Nombramos nuestra cola de procesos de baja prioridad
static struct queue *cola_baja_prioridad;
//Nombramos nuestra cola de procesos de alta prioridad
static struct queue *cola_alta_prioridad;

static char debug = 0; //Si esta activado habra trazabilidad
void imprimir(TCB *tcb) {
	if (!debug) return;

	printf("tid=%d", tcb->tid);

	printf(", state=");
	switch (tcb->state) {
		case 0:
		printf("FREE");
		break;
		case 1:
		printf("INIT");
		break;
		case 2:
		printf("WAITING");
		break;
		case 3:
		printf("IDLE");
		break;
		case 4:
		printf("RUNNING");
		break;
	}

	printf(", priority=");
	switch (tcb->priority) {
		case 0:
		printf("LOW_PRIORITY");
		break;
		case 1:
		printf("HIGH_PRIORITY");
		break;
		case 2:
		printf("SYSTEM");
		break;
	}

	printf(", ticks=%d\n\n", tcb->ticks);
}


/* Initialize the thread library */
void init_mythreadlib() {
	int i;
	/* Create context for the idle thread */
	if(getcontext(&idle.run_env) == -1){
		perror("*** ERROR: getcontext in init_thread_lib");
		exit(-1);
	}
	idle.state = IDLE;
	idle.priority = SYSTEM;
	idle.function = idle_function;
	idle.run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));
	idle.tid = -1;
	if(idle.run_env.uc_stack.ss_sp == NULL){
		printf("*** ERROR: thread failed to get stack space\n");
		exit(-1);
	}
	idle.run_env.uc_stack.ss_size = STACKSIZE;
	idle.run_env.uc_stack.ss_flags = 0;
	idle.ticks = QUANTUM_TICKS;
	makecontext(&idle.run_env, idle_function, 1);

	t_state[0].state = INIT;
	t_state[0].priority = LOW_PRIORITY;
	t_state[0].ticks = QUANTUM_TICKS;
	if(getcontext(&t_state[0].run_env) == -1){
		perror("*** ERROR: getcontext in init_thread_lib");
		exit(5);
	}

	for(i=1; i<N; i++){
		t_state[i].state = FREE;
	}

	t_state[0].tid = 0;
	running = &t_state[0];

	//Inicializar las colas de procesos
	cola_baja_prioridad = queue_new();
	cola_alta_prioridad = queue_new();

	/* Initialize disk and clock interrupts */
	init_disk_interrupt();
	init_interrupt();
}

/* Create and intialize a new thread with body fun_addr and one integer argument */
int mythread_create (void (*fun_addr)(),int priority) {
	int i;

	if (!init) { init_mythreadlib(); init=1;}
	for (i=0; i<N; i++)
		if (t_state[i].state == FREE) break;
	if (i == N) return(-1);
	if(getcontext(&t_state[i].run_env) == -1){
		perror("*** ERROR: getcontext in my_thread_create");
		exit(-1);
	}
	//Estado de thread iniciado para poder distinguir que todavia no ha terminado
	t_state[i].state = INIT;
	t_state[i].priority = priority;
	t_state[i].function = fun_addr;
	//Asignamos los cuantos de reloj definidos arriba
	t_state[i].ticks = QUANTUM_TICKS;
	t_state[i].run_env.uc_stack.ss_sp = (void *)(malloc(STACKSIZE));
	if(t_state[i].run_env.uc_stack.ss_sp == NULL){
		printf("*** ERROR: thread failed to get stack space\n");
		exit(-1);
	}
	t_state[i].tid = i;
	t_state[i].run_env.uc_stack.ss_size = STACKSIZE;
	t_state[i].run_env.uc_stack.ss_flags = 0;
	makecontext(&t_state[i].run_env, fun_addr, 1);

	//Distinguimos entre alta prioridad y baja prioridad
	if (t_state[i].priority == HIGH_PRIORITY) {
		enqueue(cola_alta_prioridad, &t_state[i]);
	} else {
		enqueue(cola_baja_prioridad, &t_state[i]);
	}

	if (debug) {
		printf("encolo:\n");
		imprimir(&t_state[i]);
	}
	//Necesitamos distinguir el proceso entre alta y baja prioridad
	if ((running->priority == LOW_PRIORITY) &&
		(t_state[i].priority == HIGH_PRIORITY)) {
		running->ticks = QUANTUM_TICKS;
		activator(scheduler());
	}

	return i;
} /****** End my_thread_create() ******/

/* Read disk syscall */
int read_disk()
{
	return 1;
}

/* Disk interrupt  */
void disk_interrupt(int sig)
{
}


/* Free terminated thread and exits */
void mythread_exit() {
	int tid = mythread_gettid();

	printf("*** THREAD %d FINISHED\n", tid);
	//Cuando el thread termina lo distinguimos por el estado FREE
	t_state[tid].state = FREE;
	free(t_state[tid].run_env.uc_stack.ss_sp);

	TCB* next = scheduler();
	activator(next);
}

/* Sets the priority of the calling thread */
void mythread_setpriority(int priority) {
	int tid = mythread_gettid();
	t_state[tid].priority = priority;
}

/* Returns the priority of the calling thread */
int mythread_getpriority(int priority) {
	int tid = mythread_gettid();
	return t_state[tid].priority;
}


/* Get the current thread id.  */
int mythread_gettid(){
	if (!init) { init_mythreadlib(); init=1;}
	return running->tid;
}


/* Planificador de tipo ROUND-ROBIN */
TCB* scheduler() {
	if ((queue_empty(cola_baja_prioridad)) &&
		(running->state == FREE) &&
		(queue_empty(cola_alta_prioridad))){
		printf("*** FINISH\n");
		exit(1);
	}

	TCB* siguiente;
	//Si actual no es FREE y es de BAJA prioridad, encolamos
	if ((running->state != FREE) && (running->priority == LOW_PRIORITY)) {
		disable_interrupt();
		enqueue(cola_baja_prioridad, running);
		enable_interrupt();
	}
	//Siempre desencolamos primero de alta prioridad
	if (!queue_empty(cola_alta_prioridad)) {
		disable_interrupt();
		siguiente = dequeue(cola_alta_prioridad);
		enable_interrupt();
	} else {
		disable_interrupt();
		siguiente = dequeue(cola_baja_prioridad);
		enable_interrupt();
	}

	if (debug) {
		printf("el scheduler selecciona:\n");
		imprimir(siguiente);
	}

	return siguiente;
}


/* Timer interrupt  */
void timer_interrupt(int sig) {
	//Con cada interrupción se reduce el número de ticks restantes
	if (running->priority == LOW_PRIORITY) running->ticks--;
	//Si se acaban los ticks estimamos que el proceso debe salir
	if ((running->ticks == 0) && (running->priority == LOW_PRIORITY)) {
		if (debug) {
			printf("quantum acabado de:\n");
			imprimir(running);
		}
		running->ticks = QUANTUM_TICKS;
		//El scheduler nos dirá cual es el siguiente proceso a ejecutar
		activator(scheduler());
	}
}

/* Activator */
void activator(TCB* next){
	TCB *previous = running;
	running = next;

	//Mismo proceso no cambia de contexto consigo mismo
	if (previous->tid == next->tid) {
		return;
	//Caso de entrada de un hilo de alta prioridad y el anterior de baja, expulsado
	} else if ((next->priority == HIGH_PRIORITY) && (previous->priority == LOW_PRIORITY)) {
		printf("*** THREAD %i PREEMTED: SETCONTEXT OF %i\n", previous->tid, next->tid);
		swapcontext(&(previous->run_env), &(next->run_env));
	//Cambiamos contexto entre dos hilos de baja prioridad
	}else if ((previous->state == INIT) && (previous->priority == LOW_PRIORITY)) {
		printf("*** SWAPCONTEXT FROM %i TO %i\n", previous->tid, next->tid);
		swapcontext(&(previous->run_env), &(next->run_env));
	//Solo ponemos el contexto cuando el ultimo hilo es de baja prioridad
	} else if (previous->state == FREE) {
		printf("*** THREAD %i TERMINATED: SETCONTEXT OF %i\n", previous->tid, next->tid);
		setcontext (&(next->run_env));
	}
}
