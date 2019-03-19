#include <stdio.h>
#include <sys/time.h>
#include <signal.h>
#include <stdlib.h>
#include <ucontext.h>
#include <unistd.h>

#include "mythread.h"


void fun1 (int global_index)
{
  int a=0, b=0;
  read_disk();
  for (a=0; a<10; ++a) {
//    printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<25000000; ++b);
  }

  for (a=0; a<10; ++a) {
//    printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<25000000; ++b);
  }
  mythread_exit();
  return;
}

void test1() {
    //printf("test1\n");
  long a = 2323232;
  for (int i = 0; i < 65500; i++){
    a *= 2;
    for (int i = 0; i < 65500; i++){
      a *= 2;
    }
  }
  mythread_exit();
  return;
}

void test2() {
  long a = 2323232;
  for (int i = 0; i < 30000; i++){
    a *= 2;
    for (int i = 0; i < 30000; i++){
      a *= 2;
    }
  }
  mythread_exit();
  return;
}

void test3() {
  long a = 2323232;
  for (int i = 0; i < 15000; i++){
    a *= 2;
    for (int i = 0; i < 15000; i++){
      a *= 2;
    }
  }
  mythread_exit();
  return;
}

void prueba_P1() {
    mythread_setpriority(LOW_PRIORITY);
    if(mythread_create(test1, LOW_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
    if(mythread_create(test2, LOW_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
}

void prueba_P2() {
    mythread_setpriority(LOW_PRIORITY);
    if(mythread_create(test1, LOW_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
    if(mythread_create(test2, LOW_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
    if(mythread_create(test2, HIGH_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
}

void prueba_P3() {
    mythread_setpriority(HIGH_PRIORITY);
    if(mythread_create(test1, HIGH_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
}

void prueba_P4() {
    mythread_setpriority(HIGH_PRIORITY);
    if(mythread_create(fun1, HIGH_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
    if(mythread_create(test1, LOW_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
    if(mythread_create(test2, LOW_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
}

void prueba_P6() {
    mythread_setpriority(HIGH_PRIORITY);
    if(mythread_create(fun1, HIGH_PRIORITY) == -1){
      printf("fallo al crear un hilo\n");
      exit(-1);
    }
}

int main(int argc, char *argv[])
{
    /* Round-Robin */
    //prueba_P1();
    /* Round-Robin/FIFO */
    //prueba_P2();
    //prueba_P3();
    /* Round-Robin/FIFO con cambios de contexto voluntarios */
    //prueba_P4();
    //prueba_P6();

    mythread_exit();

    printf("This program should never come here\n");

    return 0;
} /****** End main() ******/
