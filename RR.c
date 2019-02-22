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
//static int current = 0;

/* Variable indicating if the library is initialized (init == 1) or not (init == 0) */
static int init=0;

/* Thread control block for the idle thread */
static TCB idle;
static void idle_function(){
	while(1);
}

//Nuestras cosas
static struct queue *cola;
static char debug=1;//Si esta activado habra trazabilidad
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

	cola=queue_new();

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
	t_state[i].state = INIT;
	t_state[i].priority = priority;
	t_state[i].function = fun_addr;
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
	
	//Al crear el thread es encolado para poder ser procesado
	enqueue(cola, &t_state[i]);
	
	if (debug) {
		printf("encolo:\n");
		imprimir(&t_state[i]);
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
	t_state[tid].state = FREE;
	queue_find_remove(cola, &t_state[tid]);//TODO
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


/* FIFO para alta prioridad, RR para baja*/
//Devuelve NULL si el hilo en ejecucion es el mismo que el siguiente
TCB* scheduler() {
	/*int i;
	for(i=0; i<N; i++){
		if (t_state[i].state == INIT && t_state[i].tid!=current) {
				current = i;
			return &t_state[i];
		}
	}*/
	TCB* siguiente;
	//Mientras queden hilos por procesar ejecutamos
	while (!queue_empty(cola)){
		//Obtenemos el primer elemento sin eliminarlo
		siguiente=((TCB*)queue_front(cola));
		if (running->tid == siguiente->tid) {
			if (debug) {
				printf("el scheduler selecciona el mismo que estaba en ejecucion:\n");
				imprimir(siguiente);
			}
			return NULL;//TODO
		}
		//Ponemos al final el siguiente hilo a ejecutar puesto que ahora es su turno
		siguiente=((TCB*)dequeue(cola));
		enqueue(cola, siguiente);

		if (debug) {
			printf("el scheduler selecciona:\n");
			imprimir(siguiente);
		}
		siguiente->state=IDLE;
		return siguiente;
	}
	printf("mythread_free: No thread in the system\nExiting...\n");
	exit(1);
}


/* Timer interrupt  */
void timer_interrupt(int sig) {
	//Con cada interrupción se reduce el número de ticks restantes
	running->ticks--;
	//Si se acaban los ticks estimamos que el proceso debe salir
	if (running->ticks == 0) {
		if (debug) {
			printf("quantum acabado de:\n");
			imprimir(running);
		}
		running->ticks = QUANTUM_TICKS;
		//El scheduler nos dirá cual es el siguiente proceso a ejecutar
		TCB* siguiente = scheduler();
		if (siguiente != NULL) {
			//Si el scheduler devuelve != NULL el proceso en ejecución cambia
			running->state = WAITING;
			//Hace el cambio de contexto efectivo despues de cambiar el estado del thread actual a "ESPERANDO"
			activator(siguiente);
		}
	}
}

/* Activator */
void activator(TCB* next){
	//El thread en ejecución pasa a ser el siguiente dentro de la cola
	running=next;
	setcontext (&(next->run_env));
	printf("mythread_free: After setcontext, should never get here!!...\n");
}
