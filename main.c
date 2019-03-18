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

void test1() {
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

}

/*void prueba_P1() {

}

void prueba_P1() {

}

void prueba_P1() {

}

void prueba_P1() {

}

void prueba_P1() {

}*/

int main(int argc, char *argv[])
{
    mythread_setpriority(HIGH_PRIORITY);

    //prueba_P1();
    prueba_P2();

    mythread_exit();

    printf("This program should never come here\n");

    return 0;
} /****** End main() ******/
