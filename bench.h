#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#define KSIZE (16)
#define VSIZE (1000)

#define LINE "+-----------------------------+----------------+------------------------------+-------------------+\n"
#define LINE1 "---------------------------------------------------------------------------------------------------\n"

long long get_ustime_sec(void);
void _random_key(char *key,int length);

pthread_mutex_t write_mut;  //oi global pou xrisimopoiountai
double sumcost_writes;
pthread_mutex_t read_mut;
double sumcost_reads;

typedef struct dat  //arxikopoihsh tou struct pou xrisimopoieitai
{   int datar;
    long int datacount;
    int datathreads;
}data;