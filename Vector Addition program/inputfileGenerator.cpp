#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include <cstdlib>
 using namespace std;

 int main(int argc, char * arg[])
 {
 	FILE * outputfilePointer = fopen(arg[1], "w");
 	int n ;
 	scanf("%d", &n);
 	fprintf(outputfilePointer, "%d\n", n);
 	for(int i=0;i<n;i++)
 	{
 		int x  = rand();
 		fprintf(outputfilePointer, "%d ", x);
 	}
 	for(int i=0;i<n;i++)
 	{
 		int x  = rand();
 		fprintf(outputfilePointer, "%d ", x);
 	}
 	fclose(outputfilePointer);
 return 0;
 }