// ----------------------------------------------------------- 
// NAME : Erik Skogfeldt                     User ID: easkogfe 
// DUE DATE : 2/25/2017                                       
// PROGRAM ASSIGNMENT #2                                        
// FILE NAME : merge.c            
// PROGRAM PURPOSE :                                           
//    merges two sorted arrays stored in one shared memory 
//    segment.The merged array overwrites the original 2  
//    arrays (x and y). x[0] is first location in shared memory
//	  and y[0] is argv[2] index in shared memory	
// ----------------------------------------------------------- 
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>


// ----------------------------------------------------------- 
// FUNCTION  main :                          
//    sorts two arrays in shared memory with a merge                            
// PARAMETER USAGE :                                           
//    argc - number of string arguments passed into argv
//    argv[0] - program name              
//    argv[1] - length of array in shared memory to merge              
//    argv[2] - the index of y index in the shared memory
//    argv[3] - memory key to attach to shared memory        
// FUNCTION CALLED :                                           
//              
// ----------------------------------------------------------- 
int main(int argc, char const *argv[])
{
	int i,isParent,indexToSort,numberToSort,*interested,interestedKey,*sharedMem,length,yIndex;
	key_t key1 = ftok("./",'c');
	char buf[80];
	for (i = 0 ; i < 80 ; i++) *(buf+i) = 0; 

	length = atoi(argv[1]);
	interestedKey = shmget( key1,(sizeof(int)*length), IPC_CREAT | 0666);

	sharedMem = (int*)shmat(atoi(argv[3]),NULL,0);
	if((long)sharedMem < 0){
		perror("mem failed");
		exit(1);
	}
	yIndex = atoi(argv[2]);
	 
	interested = (int*)shmat(interestedKey,NULL,0);
	if((long)interested < 0){
		perror("interested failed");
		exit(1);
	}
	isParent = 1;
	indexToSort = -1;
	for (i=0 ; i<length ; i++)	*(interested+i) = 0;

	for(i=0 ; i<length ; i++ )
	{
		if(fork()==0)
		{
			indexToSort = i;
			isParent = 0;
			break;
		}
	}
	if (isParent == 0)
	{
		numberToSort = *(sharedMem+indexToSort);

		if(indexToSort < yIndex)
		{
			sprintf(buf,"      $$$ M-PROC(%d): handling x[%d] = %d\n",getpid(),indexToSort,numberToSort);
			write(1,buf,strlen(buf));
		}else{
			sprintf(buf,"      $$$ M-PROC(%d): handling y[%d] = %d\n",getpid(),indexToSort-yIndex,numberToSort);
			write(1,buf,strlen(buf));
		}
		
		//conduct binary search

		//index is in y array
		if (indexToSort >= yIndex)
		{
			if (numberToSort > *(sharedMem+yIndex-1) )
			{
				//position is good in y array
				sprintf(buf,"      $$$ M-PROC(%d): y[%d] = %d is found to be larger than x[%d] = %d\n",getpid(), indexToSort, numberToSort, yIndex-1, *(sharedMem+yIndex-1) );
				write(1,buf,strlen(buf));
				*(interested+indexToSort) = 1;

			}else if (numberToSort < *(sharedMem) )
			{
				//position is front of entire x array
				sprintf(buf,"      $$$ M-PROC(%d): y[%d] = %d is found to be smaller than x[0] = %d\n",getpid(), indexToSort, numberToSort, *(sharedMem)); 
				write(1,buf,strlen(buf));
				*(interested+indexToSort) = 1;
				indexToSort = (indexToSort-yIndex);

			}else
			{
				//position is in the middle on x[0] and x[m-1]
				int temp = 0;
				int L,R;
				L = 0;
				R = yIndex-1;
				temp = (R + L)/2;
				//perform binary search for index
				for (i=0; i<length ; i++)
				{
					if(*(sharedMem+temp) > numberToSort) R = temp;
					if(*(sharedMem+temp) < numberToSort) L = temp;

					if(L > R)
					{
						temp = -1;
						break;
					} 
					if(R-L <= 1)
					{
						temp = L + 1;
						break;
					}
					temp = (R + L)/2;

				}

				sprintf(buf,"      $$$ M-PROC(%d): y[%d] = %d is found between x[%d] = %d and x[%d] = %d\n",getpid(), indexToSort, numberToSort, temp-1, *(sharedMem+temp-1),temp, *(sharedMem+temp) );
				write(1,buf,strlen(buf));
				*(interested+indexToSort) = 1;
				indexToSort = (temp + (indexToSort-yIndex));

			}
		}else//index is in x array
		{

			if ( numberToSort < *(sharedMem+yIndex) )
			{
				//position is good in x array
				sprintf(buf,"      $$$ M-PROC(%d): x[%d] = %d is found to be smaller than y[0] = %d\n",getpid(), indexToSort, numberToSort, *(sharedMem+yIndex)); 
				write(1,buf,strlen(buf));
				*(interested+indexToSort) = 1;

			}else if (numberToSort > *(sharedMem+length-1) )
			{
				//position is behind entire y array
				sprintf(buf,"      $$$ M-PROC(%d): x[%d] = %d is found to be larger than y[%d] = %d\n",getpid(), indexToSort, numberToSort, length-(yIndex + 1), *(sharedMem+length-1) );
				write(1,buf,strlen(buf));
				*(interested+indexToSort) = 1;
				indexToSort += (length-yIndex);

			}else
			{
				//position is in the middle on y[0] and y[n-1]
				int temp = 0;
				int L,R;
				L = yIndex;
				R = length-1;
				temp = (R + L)/2;
				//perform binary search for index
				for (i=0; i<length ; i++)
				{
					if(*(sharedMem+temp) > numberToSort) R = temp;
					if(*(sharedMem+temp) < numberToSort) L = temp;

					if(L > R)
					{
						temp = -1;
						break;
					} 
					if(R-L <= 1)
					{
						temp = L-yIndex + 1;
						break;
					}
					temp = (R + L)/2;
				}

				sprintf(buf,"      $$$ M-PROC(%d): x[%d] = %d is found between y[%d] = %d and y[%d] = %d\n",getpid(), indexToSort, numberToSort, temp-1, *(sharedMem+temp-1+yIndex),temp, *(sharedMem+temp+yIndex) );
				write(1,buf,strlen(buf));
				*(interested+indexToSort) = 1;
				indexToSort = (indexToSort+temp);

			}
		}
	}else{ //parent waits for all children to finish
		while(wait(NULL) > 0){}
		shmdt((void*)interested);
		shmctl(interestedKey,IPC_RMID,NULL);
		exit(0);
	}

	//prevent all child processes from advancing until all child siblings have found their sorting index
	for(i=0 ; i<length ; i++) 
	{
		if( *(interested + i) == 0 ) i = -1;	
	}

	
	//children set their number into the correct sorted index
	*(sharedMem+indexToSort) = numberToSort;

	return 0;

}
