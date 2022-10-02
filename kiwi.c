#include <string.h>
#include "../engine/db.h"
#include "../engine/variant.h"
#include "bench.h"

#define DATAS ("testdb")

DB* db;

void openDB()   //orismos sinartisis anoigmatos basis dedomenwn
{	
	db = db_open(DATAS);
}
void closeDB()   //orismos sinartisis kleisimo basis dedomenwn
{
	db_close(db);
}

long int counting(long int count , int threads){ //boi8itiki sinartisi ipologismou twn leitourgiwn pou xrisomopoiountai (oles oi leitourgies/nimata ara poses epanalipseis 8a ginoun)
        int counter;
	counter = count/threads;
	return counter;
}

void read_cost(long int cost){     //boi8itiki sinartisi me mutex lock gia tin global pou exei ta kosti twn reads,pros8etei ta nea kosti sta idi iparxonta
	pthread_mutex_lock(&read_mut);
	sumcost_reads=sumcost_reads+cost;
	pthread_mutex_unlock(&read_mut);

}
void write_cost(long int cost){   //boi8itiki sinartisi me mutex lock gia tin global pou exei ta kosti twn writes,pros8etei ta nea kosti sta idi iparxonta
	pthread_mutex_lock(&write_mut);
	sumcost_writes=sumcost_writes+cost;
	pthread_mutex_unlock(&write_mut);
}



void _write_test(long int count, int r, int threads)
{
        long int counter;
	int i;
        double cost;
     	long long start,end;
	

	Variant sk, sv;
	
	char key[KSIZE + 1];
	char val[VSIZE + 1];
	char sbuf[1024];

	memset(key, 0, KSIZE + 1);
	memset(val, 0, VSIZE + 1);
	memset(sbuf, 0, 1024);

	start = get_ustime_sec(); //arxizei o metritis na metraei xrono
	
        counter = counting(count,threads); //kaleitai i sinartisi gia na broume to swsto ari8mo twn epanalipsewn

	for (i = 0; i < counter; i++) {
		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d adding %s\n", i, key);
		snprintf(val, VSIZE, "val-%d", i);

		sk.length = KSIZE;
		sk.mem = key;
		sv.length = VSIZE;
		sv.mem = val;

		db_add(db, &sk, &sv);
		if ((i % 10000) == 0) {
			fprintf(stderr,"random write finished %d ops%30s\r", 
					i, 
					"");

			fflush(stderr);
		}
	}
	end = get_ustime_sec(); //teleiwnei o metritis
	cost = end -start;   //afaireitai to telos kai i arxi kai bgainei to kostos
	
	write_cost(cost);   //to kostos mpainei stin sinartisi gia na proste8ei sto oliko global kostos
}

void _read_test(long int count, int r, int threads)
{
        long int counter;
	int found = 0;
	int i;
	double cost;
        long long start,end;
        int ret;

	Variant sk;
	Variant sv;
	char key[KSIZE + 1];

	start = get_ustime_sec();   //start/end xrisimopoiountai ida me panw
        counter = counting(count,threads);
	for (i = 0; i < counter; i++) {
		memset(key, 0, KSIZE + 1);

		if (r)
			_random_key(key, KSIZE);
		else
			snprintf(key, KSIZE, "key-%d", i);
		fprintf(stderr, "%d searching %s\n", i, key);
		sk.length = KSIZE;
		sk.mem = key;
		
		ret = db_get(db, &sk, &sv);
		if (ret) {
			found++;
		} else {
			INFO("not found key#%s", 
					sk.mem);
    	}
		if ((i % 10000) == 0) {
			fprintf(stderr,"random read finished %d ops%30s\r", 
					i, 
					"");
			fflush(stderr);
		}
	}

	end = get_ustime_sec();
	cost = end - start;

	read_cost(cost);

}
