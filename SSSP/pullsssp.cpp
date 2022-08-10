#include<bits/stdc++.h>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<CL/cl.h>
#include "Graph2.hpp"
using namespace std;

class myComparator
{
public:
    int operator() (pair<int, int> &a, pair<int, int> &b)
	{
	 	return a.first>b.first;
	}
};

void seqSSSP(int source,int *vertex_index, int *vertex_neighbour, int *sssp, int n)
{
	bool *visited = (bool *)malloc(n*sizeof(bool));
	memset(visited, false, n*sizeof(bool));
	for(int i=0; i<n; i++)
	{
		if(i==source)
		{
			sssp[i] = 0;
			continue;
		}
		sssp[i] = 1e8;
	}
	priority_queue<pair<int, int>, vector<pair<int, int>> ,  myComparator> minHeap;
    minHeap.push({0, source});
	int i=0;
	while(!minHeap.empty())
	{
		pair<int, int> y =  minHeap.top();
		minHeap.pop();
		int j = y.second;
		if(visited[j]) continue;
		visited[j] = true;
		for(int x  = vertex_index[j] ; x< vertex_index[j+1]; x++)
		{
			int dest = vertex_neighbour[x];
			if( !visited[dest] && (sssp[j] + 1< sssp[dest]))
			{
				sssp[dest] = sssp[j] +1;
				minHeap.push({sssp[dest], dest});
			}
		}
		i++;
		if(i%100000 ==0) 
		{
			cout<<"iteration no. = "<<i<<endl;
		}
	}
}

void compareResults(int *sssp, int *sequentialsssp, int n)
{
	int unmatch = 0;
	for(int i=0;i<n;i++)
	{
		if(sssp[i]!= sequentialsssp[i]) unmatch++; 
	}
	cout<<"Total number of nodes="<<n<<endl;
	cout<<"NUmber of unmatch in sequential and parallel SSSP="<<unmatch<<endl;

}


int main(int argc, char *arg[])
{
	// reading number of nodes, number of edges and source of SSSP
	int n; // number of nodes
	int e; // number of edges
	int source;// Source of SSSP
	cout<<"Enter source vertex for sssp:";
	cin>>source;
	graph G= (arg[2]);
	cout<<"Parsing graph ......."<<endl;
	G.parseGraph();
	n = G.num_nodes();
	e = G.num_edges();
	cout<<"Number of nodes = "<< n<<endl;
	cout<<"Number of edges = "<<e<<endl;
	//Compresed Sparse Row representation for GPU computation

	int *indexCSR = G.indexofNodes;
	int *CSR = G.edgeList;
	int *revIndexCSR = G.rev_indexofNodes;
	int *CSC = G.srcList;
	/*
	cout<<"revIndexCSR:"<<endl;
	for(int i=0; i<=n; i++)
	{
		cout<<revIndexCSR[i]<<" ";
	}
	cout<<"\n CSC"<<endl;
	for(int i=0; i<e; i++)
	{
		cout<<CSC[i]<<" ";
	}
	cout<<endl;

*/
cout<<"Reading kernel file..."<<endl;
// Reading the kernel file
	FILE *kernelfp = fopen(arg[1], "rb");
	char *kernelSource;
	size_t program_size;
	fseek(kernelfp, 0, SEEK_END);
	program_size = ftell(kernelfp);
	rewind(kernelfp);
	kernelSource = (char *)malloc((program_size+1)* sizeof(char));
	fread(kernelSource, sizeof(char), program_size, kernelfp);
	kernelSource[program_size] = '\0';
	fclose(kernelfp);
cout<<"Kernel file reading completed."<<endl;


cout<<"Getting platforms and devices..."<<endl;
// Getting Platform 
	cl_platform_id *platforms = NULL;
	cl_uint number_of_platforms;
	cl_int status;
	status = clGetPlatformIDs(0, NULL, &number_of_platforms);
	platforms = (cl_platform_id *)malloc(number_of_platforms*sizeof(cl_platform_id));
	status = clGetPlatformIDs(number_of_platforms, platforms, NULL);

//Getting the devices from the platform
	cl_device_id *devices= NULL;
	cl_uint number_of_devices;
	status = clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_GPU, 0, NULL, &number_of_devices);
	devices = (cl_device_id *)malloc(number_of_devices*sizeof(cl_device_id));
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, number_of_devices, devices, NULL);
	cout<<"Got platform and devices."<<endl;


// Creating context
	cl_context context;
	context = clCreateContext(NULL, number_of_devices, devices, NULL, NULL, &status);
// Create command queue
	cl_command_queue command_queue ;
	command_queue = clCreateCommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE , &status);

// Memory allocation on Host
	int *sssp = (int *)malloc(n*sizeof(int));
	int *sequentialsssp = (int*)malloc(n*sizeof(int));
	 int *changed ; // variable for identifying the change in previous iteration


// Memory allocation on device
	cl_mem vertices_device = clCreateBuffer(context, CL_MEM_READ_ONLY, (n+1)*sizeof(int), NULL, &status);
	cl_mem neighbour_device = clCreateBuffer(context, CL_MEM_READ_ONLY, (e)*sizeof(int), NULL, &status);
	cl_mem sssp_device = clCreateBuffer(context, CL_MEM_READ_WRITE, n*sizeof(int), NULL, &status);
	cl_mem changed_device = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,sizeof(int), NULL, &status);


// copy data from host to device
	status = clEnqueueWriteBuffer(command_queue, vertices_device, CL_TRUE, 0, (n+1)*sizeof(int), revIndexCSR, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(command_queue, neighbour_device, CL_TRUE,0, (e)*sizeof(int), CSC,0,NULL, NULL);
	
// Creating program from kernel sources
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, &status);

cout<<"Building program.."<<endl;
// Build the program
	status = clBuildProgram(program, number_of_devices, devices,NULL, NULL, NULL);
// create OpenCl kernel from the program
	cl_kernel kernel = clCreateKernel(program, "ssspKernelPull", &status);
	cl_kernel init = clCreateKernel(program, "initialization", &status);

// setting kernel argument 
	// kernel arguments for sssp Kernel
status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&vertices_device);
status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&neighbour_device);
status  = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&sssp_device);
status = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *)&changed_device);
status = clSetKernelArg(kernel, 4, sizeof(int), (void *)&n);


	//kernel arguments for initialization kernel
status = clSetKernelArg(init, 0, sizeof(int), (void *)&source);
status = clSetKernelArg(init, 1, sizeof(cl_mem), (void *)&sssp_device);
status = clSetKernelArg(init, 2, sizeof(int), (void *)&n);

// Events for synchronization and profiling
cl_event event1,event2,event3;

// Variables for profiling
cl_ulong start1, start2, end1, end2;
double timeInInitialization, timeInKernel=0;
cl_ulong clockPerSec = 1e6; // To convert from nano sec to ms
//Kernel setting
size_t global_size = n;
size_t local_size = 32;
cout<<"Kernel configuration completed."<<endl;

cout<<"Running initialization kernel.."<<endl;
// initialization kernel
status = clEnqueueNDRangeKernel(command_queue, init, 1, NULL, &global_size, &local_size, 0,NULL, &event1);
clWaitForEvents(1, &event1);
cout<<"initialization completed."<<endl;


//Time spent in kernel
status = clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start1, NULL);
status = clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end1, NULL);
timeInInitialization = (double)(end1-start1)/clockPerSec;


//map the 'changed' variable on host with 'changed_device' variable on device
changed = (int *)clEnqueueMapBuffer(command_queue, changed_device, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0,sizeof(int),0,NULL,NULL,&status);
cout<<"Running sssp kernel..."<<endl;
//Launch sssp kernel in do while loop
int i=0;
do{
	*changed = 0;
	status = clEnqueueNDRangeKernel(command_queue, kernel, 1,NULL, &global_size, &local_size , 0, NULL, &event3);
	clWaitForEvents(1, &event3);
	if(status != CL_SUCCESS)
	{
		cout<<"kernel execution unsuccessful. Status code:"<< status<<endl;
	}
	status = clGetEventProfilingInfo(event3, CL_PROFILING_COMMAND_START,sizeof(cl_ulong), &start2, NULL);
	status = clGetEventProfilingInfo(event3, CL_PROFILING_COMMAND_END,sizeof(cl_ulong), &end2, NULL);
	timeInKernel += (double)(end2- start2)/clockPerSec;

	i++;
}while(*changed && i<=n);
// unmap the mapped memory
clEnqueueUnmapMemObject(command_queue, changed_device, changed, 0, NULL, NULL);


cout<<"number of times kernel called = "<<i<<endl;
cout<<"Time spent in initialization = "<<timeInInitialization<<" ms."<<endl;
cout<<"Time spent in kernel = "<<timeInKernel<<" ms."<<endl;
cout<<"Total time for SSSP = "<<timeInInitialization + timeInKernel<<" ms."<<endl;

// what if graph has negative weight cycle?
if(i==n)
{
cout<<"Graph contains negative waight cycle.Can't find shortest path."<<endl;
}

// copy result to host
status = clEnqueueReadBuffer(command_queue, sssp_device , CL_TRUE, 0, n*sizeof(int), sssp, 0, NULL, &event2);
clWaitForEvents(1,&event2);

// cleanup and wait for all commands to complete
status = clFlush(command_queue);
status = clFinish(command_queue);

cout<<"calling sequential sssp...."<<endl;
// calling the sequential SSSP
seqSSSP(source,indexCSR,CSR, sequentialsssp,n);

cout<<"sequential sssp completed."<<endl;
// compare the two results from parallel and sequential SSSP
compareResults(sssp, sequentialsssp, n);


// writing the result in output file
FILE * outputfp = fopen(arg[3],"w");
for(int i=0;i<n;i++)
{
	fprintf(outputfp, "%d  %d\n",i, sssp[i] );
}
fclose(outputfp);


cout<<" OpenCL objects releasing starting"<<endl;
// release all openCL objects
status = clReleaseEvent(event1);
status = clReleaseEvent(event2);
status = clReleaseEvent(event3);
status = clReleaseKernel(init);
status = clReleaseKernel(kernel);
status = clReleaseMemObject(vertices_device);
status = clReleaseMemObject(neighbour_device);
status = clReleaseMemObject(sssp_device);
status = clReleaseMemObject(changed_device);
status = clReleaseCommandQueue(command_queue);
status = clReleaseContext(context);
free(indexCSR);
free(CSR);
free(revIndexCSR);
free(CSC);
//free(changed);
free(sssp);
free(devices);
free(platforms);
cout<<"Done!"<<endl;
return 0;
}