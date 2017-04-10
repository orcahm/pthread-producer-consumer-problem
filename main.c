#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

/*
References
http://pages.cs.wisc.edu/~remzi/Classes/537/Fall2008/Notes/threads-semaphores.txt 

CommadLine
argv[1]=create producer
argv[2]=create consumer
argv[3]=size of buffer
argv[4]=output name
*/

	/* Global variable: accessible to all threads */ 
	int producer_thread_count;
	int consumer_thread_count;
	int fill = 0; 
	int use  = 0;
	int *buffer;
	sem_t empty;
	sem_t full;
	sem_t mutex;
	int MAX;
	int buffer_count = 0;
	int consumer_count = 0;
	FILE *outputFile;
	
	void *consumer(void *rank);
	void *producer(void *rank);
	void put(int value);
	int get();
	
	int main(int argc, char* argv[]) {
	
		int i;
		long thread; 
		outputFile = fopen(argv[4] , "w");
		if (!outputFile)
        		return 0;
	
		/* Use long in case of a 64-bit system */
		pthread_t* thread_handles;
		
		MAX = strtol(argv[3], NULL, 10);
		
		buffer = malloc(sizeof(int)*MAX);
		
		for(i=0; i<MAX;i++)
			buffer[i]=-1;
		
		/* Get number of threads from command line */ 
		producer_thread_count = strtol(argv[1], NULL, 10); 
		consumer_thread_count = strtol(argv[2], NULL, 10); 
		
		thread_handles = malloc (sizeof(pthread_t)*(producer_thread_count+consumer_thread_count)); 
		
		sem_init(&empty, 0, MAX); // MAX buffers are empty to begin with...
 	  	sem_init(&full, 0, 0);    // ... and 0 are full
   		sem_init(&mutex, 0, 1);   // mutex = 1 because it is a lock (NEW LINE)
   		
   		fprintf(outputFile,"Create %d pruoducer thread, %d consumer thread and %d size of buffer\n", producer_thread_count,consumer_thread_count,MAX);  
		
		for (thread = 0; thread < producer_thread_count; thread++) 	
			pthread_create(&thread_handles[thread], NULL, producer, (void*) thread); 
			
		for (thread = producer_thread_count; thread < producer_thread_count+consumer_thread_count; thread++) 	
			pthread_create(&thread_handles[thread], NULL, consumer, (void*) thread); 
		
		for (thread = 0; thread < producer_thread_count+consumer_thread_count; thread++)  
			pthread_join(thread_handles[thread], NULL); 
		
		free(thread_handles);
			
		fclose(outputFile);			
		return 0; 
		
	} /* main */ 
	
	void *producer(void *rank) {
		srand(time(NULL));
		long my_rank = (long) rank+1;
		int i;
		sem_wait(&mutex);
	   	while(buffer_count!=MAX*5){
	   		buffer_count++;
	   	sem_post(&mutex);
	   		i=(rand()%MAX)/my_rank;
	   		sem_wait(&empty);
	        sem_wait(&mutex);
	        fprintf(outputFile,"*The %d. producer thread made %d the %d. element of buffer array.*\n" , my_rank, i, fill ); 
			put(i);  
	        sem_post(&mutex);
	        sem_post(&full);
	    }
	}
	
	void *consumer(void *rank) {
		long my_rank = (long) rank - producer_thread_count+1;
		sem_wait(&mutex);
	    while(consumer_count!=MAX*5){
	    	consumer_count++;
	    sem_post(&mutex);
	        sem_wait(&full);
	        sem_wait(&mutex);
	        int my_use=use;
	        int tmp = get();
	        sem_post(&mutex);
	        sem_post(&empty);
	        fprintf(outputFile,"!The %d. consumer thread read %d the %d. element of buffer array.!\n", my_rank, tmp, my_use);
	    }
	}

	void put(int value) {
		
		    buffer[fill] = value;    
		    fill = (fill + 1) % MAX;
		    
	}
		
	int get() {
		    int tmp = buffer[use];
		    buffer[use] = -1;
			use = (use + 1) % MAX;
		    return tmp;
	}

