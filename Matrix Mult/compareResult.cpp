#include<iostream>
#include<cstdlib>

using namespace std;

int main(int argc, char *arg[])
{
	// file pointer for input files
	FILE *fp1 = fopen(arg[1], "r");// actual result file
	FILE *fp2 = fopen(arg[2], "r"); // Expected result file
	// counter which will count how matched ot not matched.
	int total_cnt=0;
	int unmatch_cnt = 0;
	int x, y;
	while(1)
	{
		 fscanf(fp1, "%d", &x);
		fscanf(fp2, "%d", &y);
		if(feof(fp2)) break;

		if(feof(fp1))
		{
			total_cnt ++;
			unmatch_cnt ++;
			break;
		} 

		total_cnt ++;
		if(x!=y)
		{
			unmatch_cnt++;
		}
	}
	
	while(1)
	{
		fscanf(fp2, "%d", &y);
		if(feof(fp2)) break;
		total_cnt ++;
		unmatch_cnt++;
	}
	fclose(fp1);
	fclose(fp2);
	cout<<"Total count ="<< total_cnt<<endl;
	cout<<"Unmatched count = "<< unmatch_cnt<<endl;
	return 0;
}