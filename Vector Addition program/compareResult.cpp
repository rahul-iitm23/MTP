#include<iostream>
#include<stdlib.h>

using namespace std;

int main(int argc , char *arg[])
{
	FILE * inputfp = fopen(arg[1], "r");
	FILE * outputfp = fopen(arg[2], "r");
	int n;
	fscanf(inputfp, "%d", &n);
	int *A = (int *)malloc(n*sizeof(int));
	int *B = (int *)malloc(n*sizeof(int));

	for(int i=0;i<n;i++)
	{
		fscanf(inputfp, "%d ", &A[i]);
	}
	for(int i=0;i<n;i++)
	{
		fscanf(inputfp, "%d", &B[i]);
	}
	bool flag = true;
	for(int i=0;i<n;i++)
	{
		int x ;
		fscanf(outputfp, "%d", &x);
		if(A[i]+B[i] != x)
		{
			cout<< "Doesn't match\n";
			flag = false;
			break;
		}
	}
	if(flag)
	{
		cout<< "Match :)\n";
	}
return 0;
}
