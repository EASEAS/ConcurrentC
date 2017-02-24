// ----------------------------------------------------------- 
// NAME : Erik Skogfeldt                     User ID: easkogfe 
// DUE DATE : 2/25/2017                                        
// PROGRAM ASSIGNMENT #2                                        
// FILE NAME : qsort.c           
// PROGRAM PURPOSE :                                           
//    program implemenets quick sort utilizing child processes     
// ----------------------------------------------------------- 
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void swap(int *arrayToSwap, int num1, int num2);
void printIntArray(int * array, int length, int indent);

// ----------------------------------------------------------- 
// FUNCTION  main :                          
//    sorts data in shared memory with a quick sort                            
// PARAMETER USAGE :                                           
//    argc - number of string arguments passed into argv
//    argv[0] - program name              
//    argv[1] - left index of array to sort              
//    argv[2] - right index of array to sort
//    argv[3] - memory key to attach to shared memory        
// FUNCTION CALLED :                                           
//    swap() printIntArray()
// ----------------------------------------------------------- 
int main(int argc, char const *argv[])
{

	int *memID,Lindex,Rindex,pivot,pivotIndex,i,tempR,tempL;
	char buf[80];
	char spacer[4]; 
	sprintf(spacer,"     "); 
	Lindex = atoi(argv[1]);
	Rindex = atoi(argv[2]);
	tempL = Lindex + 1;
	tempR = Rindex;
	memID = (int*)shmat(atoi(argv[3]),NULL,0);
	pivotIndex = ((Rindex-Lindex)/2) + Lindex;

	for (i = 0 ; i < 80 ; i++) *(buf+i) = 0; 
	if((long)memID<0)
	{
		perror("Memory failed to attach");
		exit(1);
	}

	sprintf(buf,"   ### PROC(%d): entering with a[%d..%d]\n", getpid(),Lindex,Rindex);
	write(1,buf,strlen(buf));
	printIntArray(memID+Lindex, (Rindex-Lindex) + 1,2);

	if((Rindex-Lindex) == 1)
	{ 
		//end case when partitions are 2 elements
		if ( *(memID+Lindex) >  *(memID+Rindex) ) swap(memID, Lindex, Rindex);
		sprintf(buf,"   ### Q-PROC(%d): exits\n",getpid());
		write(1,buf,strlen(buf));
		exit(0);
	}
	
	pivot = *(memID+pivotIndex);
	sprintf(buf,"   ### Q-PROC(%d): pivot element is a[%d] = %d\n", getpid(), pivotIndex, pivot);
	write(1,buf,strlen(buf));

	//change array to allow in place partitioning
	swap(memID , pivotIndex, Lindex);

	//perform in place partition-ing
	while(tempR > tempL)
	{
		while(tempL <= Rindex && *(memID+tempL) < pivot) tempL++;

		while(tempR > Lindex && *(memID+tempR) >= pivot) tempR--;

		if( tempR>tempL ){
			swap(memID, tempR, tempL);
		}
	}
	//the tempR is in a position which is also the index where pivot should be located
	*(memID + Lindex) = *(memID + tempR);
	*(memID + tempR) = pivot;


	sprintf(buf,"   ### PROC(%d): section a[%d..%d] sorted\n", getpid(),Lindex,Rindex);
	write(1,buf,strlen(buf));

	printIntArray(memID+Lindex,  (Rindex-Lindex) + 1, 2);

	if(Lindex<tempR-1 && fork() == 0)
	{
		char *arguments[5];
		char  L [4];
		char  R [4];
		char  memIDShare [4];

		sprintf(L,"%d",Lindex);
		sprintf(R,"%d",(tempR-1));
		sprintf(memIDShare,"%d",atoi(argv[3]));

		arguments[0] = "./qsort";
		arguments[1] = L;	//left index
		arguments[2] = R;	//right index
		arguments[3] = memIDShare;

		arguments[4] = NULL; 

		execvp(arguments[0],arguments);
		perror("exec failed\n");
		exit(1);
	}	
	if(tempR+1<Rindex && fork() == 0)
	{
		char *arguments[5];
		char  L [4];
		char  R [4];
		char  memIDShare [4];

		sprintf(L,"%d",(tempR+1));
		sprintf(R,"%d",Rindex);
		sprintf(memIDShare,"%d",atoi(argv[3]));

		arguments[0] = "./qsort";
		arguments[1] = L;	//left index
		arguments[2] = R;	//right index
		arguments[3] = memIDShare;

		arguments[4] = NULL; 

		execvp(arguments[0],arguments);
		perror("exec failed\n");
		exit(1);
	}
	wait(NULL);
	wait(NULL);
	
	return 0;
}
// ----------------------------------------------------------- 
// FUNCTION  swap :                         
//     swaps two values in an array                           
// PARAMETER USAGE :                                           
//    int *arrayToSwap- pointer to array to have values swapped
//	  int num1- position in array to swap with num2
//    int num2- position in array to swap with num1      
// FUNCTION CALLED :                                           
//  	
// -----------------------------------------------------------
void swap(int *arrayToSwap, int num1, int num2)
{
	int temp = *(arrayToSwap + num1 );
	*(arrayToSwap + num1 ) =  *(arrayToSwap + num2)  ;
	*(arrayToSwap + num2 ) = temp;
}

// ----------------------------------------------------------- 
// FUNCTION  printIntArray :                         
//     makes buffered integer writes to stdout.                           
// PARAMETER USAGE :                                           
//    int *array- pointer to array to have values printed
//	  int length- number of element to print from int* array
//    int indent- number of spaces*3 to prefix buffered output      
// FUNCTION CALLED :                                           
//  	
// -----------------------------------------------------------
void printIntArray(int * array, int length, int indent)
{
	int i;
	char buf [80];
	char spacer[4]; 
	sprintf(spacer,"   "); 
	for (i = 0 ; i < 80 ; i++) *(buf+i) = 0;
	for (i = 0 ; i < indent ; i++) 	strcat(buf, spacer); 
	for (i=0 ; i<length ; i++ )
	{
		
		char temp[4];
		if ( i%(20 - indent)==0 && i != 0)
		{
			int j;
			sprintf(temp,"\n"); 
			strcat(buf, temp);
			write(1,buf,strlen(buf));
			for (j = 0 ; j < 80 ; j++) *(buf+j) = 0; 
			for (j = 0 ; j < indent ; j++) 	strcat(buf, spacer); 

		}
		sprintf(temp,"%4d",*(array+i)); 
		strcat(buf, temp);
		if(i == length -1 ){
			sprintf(temp,"\n"); 
			strcat(buf, temp);
			write(1,buf,strlen(buf));
		}

	}
}


