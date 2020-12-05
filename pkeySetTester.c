#define _GNU_SOURCE

#include<stdio.h>
#include<sys/mman.h>
#include<time.h>
#include<stdlib.h>

#define PAGESIZE 4096

void errExit( const char * t){
    printf("\n%s\n",t);
    exit(-1);
}
static long get_nanos() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

#define memSize 4294967296      //4G

void main(void) {
    int status, pkey, counter = 0;
    void *buffer, *mprotectAddr;
    void *tail;
	long timeBegin, timeEnd;
    unsigned long long mask;

    int cpySize[26] = {3,13,50,100,512,1024};
    for(int i=6;i<26;i++){
        cpySize[i] = cpySize[i-1]+4096;
    }
    mask = ~(4096-1);

    buffer = calloc(1, memSize);
    if (buffer <0)
        errExit("malloc\n");

    pkey = pkey_alloc(0, PKEY_DISABLE_ACCESS);
    if (pkey == -1)
        errExit("pkey_alloc");

    tail =(void*)((unsigned long)buffer + memSize - 6245 - cpySize[25]);

    printf("time(ns)\n");

	timeBegin = get_nanos();
	for(int i=0;i<2000;i++){
		counter++;
        if(i%2==0){
            pkey_set(pkey, 0);
        }else{
            pkey_set(pkey, PKEY_DISABLE_ACCESS);
        }
	}
	timeEnd = get_nanos();
	printf("\tpkey_set time %ld, set %d times, everage set time %ld\n\n",
			timeEnd-timeBegin, counter, (timeEnd-timeBegin)/counter);

    for(int j=0;j<26;j++){
        int cmpSize = cpySize[j];
        counter = 0;
        mprotectAddr = buffer;
    	timeBegin = get_nanos();
    	for(;(unsigned long)mprotectAddr<=(unsigned long)tail;
               mprotectAddr = mprotectAddr+cmpSize+4097){
            if( pkey_mprotect((void*)((unsigned long long)mprotectAddr&mask), cmpSize,
                           PROT_READ | PROT_WRITE, pkey)){
                printf("pkey_mprotect error\n");
            }
    		counter++;
    	}
    	timeEnd = get_nanos();
    	printf("\tpkey_mprotect consume time %10ld, ", timeEnd-timeBegin);
    	printf("pkey_mprotect %10d times, ", counter);
    	printf("everage set time %6ld, ", (timeEnd-timeBegin)/counter);
    	printf("pkey_mprotect size %7d\n", cmpSize);
    }

    status = pkey_free(pkey);
    if (status == -1)
        errExit("pkey_free");

    return ;
}
