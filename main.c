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


void fun2 (int global_index)
{
  int a=0, b=0;
  read_disk();
  for (a=0; a<10; ++a) {
  //  printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<18000000; ++b);
  }
  for (a=0; a<10; ++a) {
  //  printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<18000000; ++b);
  }
  mythread_exit();
  return;
}

void fun3 (int global_index)
{
  int a=0, b=0;
  for (a=0; a<10; ++a) {
    //printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<40000000; ++b);
  }
  for (a=0; a<10; ++a) {
    //printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
    for (b=0; b<40000000; ++b);
  }
  mythread_exit();
  return;
}


void crear_hilos(int prueba) {
    switch (prueba) {
        case 0:
            read_disk();
            if(mythread_create(fun1,LOW_PRIORITY) == -1){
              printf("thread failed to initialize\n");
              exit(-1);
            }
           read_disk();
            if(mythread_create(fun2,LOW_PRIORITY) == -1){
              printf("thread failed to initialize\n");
              exit(-1);
            }
            if(mythread_create(fun3,LOW_PRIORITY) == -1){
              printf("thread failed to initialize\n");
              exit(-1);
            }
            if(mythread_create(fun1,HIGH_PRIORITY) == -1){
              printf("thread failed to initialize\n");
              exit(-1);
            }

            if(mythread_create(fun2,HIGH_PRIORITY) == -1){
              printf("thread failed to initialize\n");
              exit(-1);
            }

            for (int a=0; a<10; ++a) {
            //    printf ("Thread %d with priority %d\t from fun2 a = %d\tb = %d\n", mythread_gettid(), mythread_getpriority(), a, b);
               for (int b=0; b<30000000; ++b);
            }

            if(mythread_create(fun1,HIGH_PRIORITY) == -1){
              printf("thread failed to initialize\n");
              exit(-1);
            }
            if(mythread_create(fun1,HIGH_PRIORITY) == -1){
              printf("thread failed to initialize\n");
              exit(-1);
            }
        break;
        case 1:
            if(mythread_create(fun1,LOW_PRIORITY) == -1){
              printf("fallo al crear un hilo\n");
              exit(-1);
            }
        break;
        case 2:
            if(mythread_create(fun1,LOW_PRIORITY) == -1){
              printf("fallo al crear un hilo\n");
              exit(-1);
            }
            if(mythread_create(fun2,HIGH_PRIORITY) == -1){
              printf("fallo al crear un hilo\n");
              exit(-1);
            }
        break;
    }
}

void test1() {
  sleep(5);
  mythread_exit();
  return;
}

void test2() {
  sleep(2.5);
  mythread_exit();
  return;
}

void test3() {
  sleep(1.25);
  mythread_exit();
  return;
}

void test4() {
  sleep(10);
  mythread_exit();
  return;
}

void test5() {
  sleep(15);
  mythread_exit();
  return; 
}

void test6() {
  sleep(7.5);
  mythread_exit();
  return;
}

int main(int argc, char *argv[])
{
    mythread_setpriority(HIGH_PRIORITY);
    crear_hilos(1);
    mythread_exit();

    printf("This program should never come here\n");

    return 0;
} /****** End main() ******/
