#include "bench.h"
#include <math.h>

void _random_key(char *key,int length) {
	int i;
	char salt[36]= "abcdefghijklmnopqrstuvwxyz0123456789";

	for (i = 0; i < length; i++)
		key[i] = salt[rand() % 36];
}

void _print_header(int count)
{
	double index_size = (double)((double)(KSIZE + 8 + 1) * count) / 1048576.0;
	double data_size = (double)((double)(VSIZE + 4) * count) / 1048576.0;

	printf("Keys:\t\t%d bytes each\n", 
			KSIZE);
	printf("Values: \t%d bytes each\n", 
			VSIZE);
	printf("Entries:\t%d\n", 
			count);
	printf("IndexSize:\t%.1f MB (estimated)\n",
			index_size);
	printf("DataSize:\t%.1f MB (estimated)\n",
			data_size);

	printf(LINE1);
}

void _print_environment()
{
	time_t now = time(NULL);

	printf("Date:\t\t%s", 
			(char*)ctime(&now));

	int num_cpus = 0;
	char cpu_type[256] = {0};
	char cache_size[256] = {0};

	FILE* cpuinfo = fopen("/proc/cpuinfo", "r");
	if (cpuinfo) {
		char line[1024] = {0};
		while (fgets(line, sizeof(line), cpuinfo) != NULL) {
			const char* sep = strchr(line, ':');
			if (sep == NULL || strlen(sep) < 10)
				continue;

			char key[1024] = {0};
			char val[1024] = {0};
			strncpy(key, line, sep-1-line);
			strncpy(val, sep+1, strlen(sep)-1);
			if (strcmp("model name", key) == 0) {
				num_cpus++;
				strcpy(cpu_type, val);
			}
			else if (strcmp("cache size", key) == 0)
				strncpy(cache_size, val + 1, strlen(val) - 1);	
		}

		fclose(cpuinfo);
		printf("CPU:\t\t%d * %s", 
				num_cpus, 
				cpu_type);

		printf("CPUCache:\t%s\n", 
				cache_size);
	}
}


void *myfunc_read(void *arg)
{	data *d = (data *)arg;
	_read_test(d->datacount, d->datar, d->datathreads);  //boi8itiki sinartisi gia tin read
	return 0;
}

void *myfunc_write(void *arg)
{	data *d = (data *)arg;
	_write_test(d->datacount, d->datar, d->datathreads); //boi8itiki sinartisi gia tin write
	return 0;
}

void write_printer(long count, int threads ,float cost){  //sinartisi gia ta print twn write
	double x = cost/count;
	double y = count/cost;
	printf(LINE);
	printf("|Random-Write	(done:%ld): %.6f sec/op; %.1f writes/sec(estimated); cost:%.3f(sec);\n"
	,count, (x)
	,(double)(y)   //ola ta print twn apotelesmatwn
	,cost);
}

void read_printer(long count , int threads, float cost){    //sinartisti gia ta print twn read
	double x = cost/count;	// xronos apokrisis
	double y = count/cost;	//ru8mapodosi				
	printf(LINE);
	printf("|Random-Read	(done:%ld): %.6f sec/op; %.1f reads /sec(estimated); cost:%.3f(sec)\n",
	count, (double)(x),
	(double)(y),
	cost); //sinolikos xronos,diladi kostos tis leitourgias
}
int main(int argc,char** argv)
{
	
	int threads,i;
        long int count;
	data define, define1;


        pthread_mutex_init(&read_mut,NULL);  //arxikopoihseis twn mutex gia ta global kostoi se read/write
        sumcost_reads=0;
	
	pthread_mutex_init(&write_mut,NULL);    	
	sumcost_writes=0;
	
	
	srand(time(NULL));
	if (argc < 4 || argc > 7) { //mikrotera apo 4 den mpainei pou8ena ara error,perissotera apo 7 den ginetai akoma kai gia thn extra leitourgia + r
		fprintf(stderr,"Usage: db-bench <write | read | read||write | write> <count> <threads> <percentage> <r> \n");
		exit(1);
	}
		
	if (strcmp(argv[1], "write") == 0 && argc < 6) {
		int r = 0;
		
		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 4)
			r = 1;
		threads = atoi(argv[3]);

		pthread_t  *measurements;
		
		measurements = (pthread_t*) malloc(threads * sizeof(pthread_t)); //dinamiki desmeusi mnimis gia ta threads
		
		openDB();   //boi8itiki sinartisi gia anoigma tis basis dedomenwn

		define.datar=r;
		define.datacount=count;
		define.datathreads=threads;   

		for(i=0;i<threads;i++)
			pthread_create(&measurements[i],NULL,myfunc_write,(void *)&define); //dimiourgia twn nimatwn
		for(i=0;i<threads;i++)		                                          //perasma pollaplwn parametwn
			pthread_join(measurements[i],NULL);

		closeDB();  //boi8itiki sinartisi gia to kleisimo tis basis dedomenwn

		write_printer(define.datacount,define.datathreads,sumcost_writes);

		free(measurements);

	} else if (strcmp(argv[1], "read") == 0 && argc < 6) {  //ta comment tis read kai o tropos einai idios me tin write parapanw
		int r = 0;

		count = atoi(argv[2]);
		_print_header(count);
		_print_environment();
		if (argc == 4)
			r = 1;
		threads = atoi(argv[3]);
		
		pthread_t  *measurements;		
		measurements = (pthread_t*) malloc(threads * sizeof(pthread_t));
		
		openDB();

		define.datathreads=threads;
                define.datar=r;
		define.datacount=count;

		for(i=0;i<threads;i++)
			pthread_create(&measurements[i],NULL,myfunc_read,(void *)&define);
		for(i=0;i<threads;i++)
			pthread_join(measurements[i],NULL);
		closeDB();
		
		read_printer(define.datacount,define.datathreads,sumcost_reads);

		free(measurements);

        // ta define gia tin parakatw leitourgia einai tis taksis ./kiwi-bench   read     write    1000      100      60        r
        // opou ta read kai write mporoun na pane antistrofa          argv[0]   argv[1]  argv[2]  argv[3]  argv[4] argv[5]  argv[6]

        // an to read einai prwto tote to pososto pou dinetai sto argv[5] paei se auto,an to write einai prwto tote to pososto paei se ekeino

	}  else if (argc == 6 || argc == 7) { //argc == 6 h argc==7 8a einai mono an 8eloume na mpoume se autin tin leitourgia
		int r = 0;
		float percent;
		long int countnumber;
		int threadsnumber;
		count = atoi(argv[3]);
		_print_header(count);
		_print_environment();

		if (argc == 6)
			r = 1;

		threads = atoi(argv[4]);  //nimata

		percent = atoi(argv[5]);   //pososto pou dinei o xristis

		threadsnumber = threads * percent/100;  // ari8mos nimatwn(pou edwse o xristis ) = nimata * pososto/100

		countnumber = count * percent/100;	//ari8mos leitourgiwn(pou edwse o xristis)  = leitourgies * pososto/100

		
		pthread_t  *measurements;		
		measurements = (pthread_t*) malloc(threads * sizeof(pthread_t));

		
		pthread_t  *measurements1;		
		measurements1 = (pthread_t*) malloc(threads * sizeof(pthread_t));
		
		openDB();

		define.datar=r;
		define.datacount=(long) (countnumber);
		define.datathreads=(int) (threadsnumber);
		define1.datar=r;
		define1.datacount=(long) (count-countnumber);
		define1.datathreads=(int) (threads-threadsnumber);

		if(strcmp(argv[1], "write") == 0){  //an dw8ike prwto to write to pososto twn nimatwn/leitourgiwn pou dinoume pigainei sto write
			
			define.datathreads=(int) (threadsnumber);  //ari8mos nimatwn
                        define.datar=r;
			define.datacount=(long) (countnumber);   //ari8mos leitourgiwn

			define1.datathreads=(int) (threads - threadsnumber); //nimata(ola osa do8ikan) - ari8mos nimatwn pou do8ikan = ta ipoloipa nimata pou menoun
                        define1.datar=r;
			define1.datacount=(long) (count-countnumber);    //leitourgies(oses do8ikan) - ari8mos leitourgiwn pou di8ikan =ipoloipapososto twn leitourgiwn 

		}else {   //an den einai 1o to write kai exei argc = 6 h argc = 7 mpike edw,ara to prwto einai read kai to pososto pou dwsame einai stin read
			  // ta parakatw einai akribws idias logikis me apo panw alla anti8eta afou pleon ta pososta pou do8ikan pane stin read

			define.datathreads=(int) (threadsnumber);
                        define.datar=r;
			define.datacount=(long) (countnumber);

			define1.datathreads=(int) (threads - threadsnumber);
                        define1.datar=r;
			define1.datacount=(long) (count - countnumber);
		} 
			//dimiourgia pollaplwn nimatwn opws se diafania ma8imatos pou ekteloun tis boi8itikes sinartiseis myfunc_write/myfunc_read

		for(i=0;i<(threadsnumber);i++)  //for gia ari8mo nimatwn
			pthread_create(&measurements[i],NULL,myfunc_write,(void *)&define);  
		for(i=0;i<(threads-threadsnumber);i++) //for gia ton ari8mo twn ipoloipwn nimatwn
			pthread_create(&measurements1[i],NULL,myfunc_read,(void *)&define1);
		for(i=0;i<(threads-threadsnumber);i++)
			pthread_join(measurements1[i],NULL);
		for(i=0;i<(threadsnumber);i++)
			pthread_join(measurements[i],NULL);
		closeDB();

		if (strcmp(argv[1], "write") == 0){ //an prwto write ta katallila print gia write
			printf(LINE);
			write_printer(define.datacount,define.datathreads,sumcost_writes);
			read_printer(define1.datacount,define1.datathreads,sumcost_reads);
		}else{   				
			printf(LINE);	// anti8eta ta parapanw print afou pleon eimaste se do8wn posostou stin read
			read_printer(define.datacount,define.datathreads,sumcost_reads);
			write_printer(define1.datacount,define1.datathreads,sumcost_writes);
		}
		free(measurements);      //apodesmeusi mnimis afou oristikan me malloc gia na mhn uparxei keno mnimis
		free(measurements1);
	} else {
		fprintf(stderr,"Usage: db-bench <write | read | <read> || <write> <count> <threads> <percent> <r> \n");
		exit(1);
	}

	return 1;
}

