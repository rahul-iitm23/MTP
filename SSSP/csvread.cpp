/* 
purpose : process the csv file containing edges of a unweighted graph and producess text
			for the SSSP program. first line of output file will contain n= number of vertices
			e = number of edges , source = source vertex for single source shortest path.
			Next e lines will contain edges.
command line arguments: first argument- file  which cotains all vertices, second argument-
						file which contains all the edges
						third argument- output file.


*/
#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<vector>
using namespace std;
vector<int> parse(char *buffer )
{
	int num=0;
	int i=0;
	vector<int> vec;
	while(buffer[i] !='\n')
	{
		if(buffer[i]==',')
		{
			vec.push_back(num);
			num=0;
		}
		else if(buffer[i]=='\n')
		{
			vec.push_back(num);
			break;
		}
		else if(buffer[i]>='0' && buffer[i]<='9')
		{
			num = num*10 + (buffer[i]-'0');
		}
		i++;
	}
	return vec;
}
int main(int argc, char *arg[])
{
	FILE * inputfp = fopen(arg[1],"r");
	FILE * outputfp = fopen(arg[3],"w");
	
	int n=-1; // for counting number of nodes
	char buffer[100];
	while(fgets(buffer, 100, inputfp))
	{
		n++;
	}
fclose(inputfp);
inputfp = fopen(arg[2],"r");
int e = -1;// number of edges
while(fgets(buffer, 100, inputfp))
{
	e++;
}
rewind(inputfp);
int source = 0; // keeping source vertex 0;
fprintf(outputfp, "%d %d %d\n",n,e,source );
vector<int> edge;
int i=0;
while(fgets(buffer, 100, inputfp))
{
	if(!i)
	{
		i=1;
		continue;
	}
	edge = parse(buffer);
	fprintf(outputfp, "%d %d\n", edge[0], edge[1]);
}
fclose(inputfp);
fclose(outputfp);


return 0;

}