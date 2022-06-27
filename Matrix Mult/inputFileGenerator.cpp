#include<iostream>
#include<cstdlib>

using namespace std;

int main(int argc, char *arg[])
{
	FILE *outputfp = fopen(arg[1], "w");
	int p, q, r;
	cout<<"Enter dimension of matrices as p, q and r:";
	cin>>p>>q>>r;
	fprintf(outputfp, "%d %d %d \n", p,q,r);
	for(int i=0;i<p;i++)
	{
		for(int j=0;j<q;j++)
		{
			int x = rand() % 1000;
			fprintf(outputfp, "%d ", x);
		}
		fprintf(outputfp, "\n");
	}
	for(int i=0;i<q;i++)
	{
		for(int j=0;j<r;j++)
		{
			int x = rand() % 1000;
			fprintf(outputfp, "%d ", x);
		}
		fprintf(outputfp, "\n");
	}

cout<<"Your output file has been generated. First line of file contains p q r.\n";
cout<<"Then from next line there are two matrix created.\n";
cout<<" First one is of size p*q and second matrix is of size q*r\n";
fclose(outputfp);
return 0;
}
