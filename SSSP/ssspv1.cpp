#include<iostream>
#include<stdio.h>
#include<vector>
#include<time.h>
#include<stdlib.h>
#include<CL/cl.h>
using namespace std;

int main(int argc, char *arg[])
{
	// file pointers for file input/output
	FILE *inputfp = fopen(arg[2], "r");
	FILE *outputfp  = fopen(arg[3],"w");
	
	// reading number of nodes, number of edges and source of SSSP

	int n; // number of nodes
	int e; // number of edges
	int source;// Source of SSSP
	fscanf(inputfp, "%d %d %d", &n, &e, &source);

	// taking graph input . Considering graph is undirected and unweighted

	vector<int> G[n]; // Graph on Host
	for(int i=0;i<e; i++)
	{
		int s,d;  
		fscanf(inputfp, " %d %d", &s, &d);
		G[s].push_back(d);
		G[d].push_back(s);
	}
	//Compresed Sparse Row representation for GPU computation

	int *vertex_index = (int *)malloc((n+1)*sizeof(int));
	int *vertex_neighbour = (int*)malloc(2*e*sizeof(int));
	int k = 0;
	for(int i=0;i<n; i++)
	{
		vertex_index[i] = k;
		for(int x: G[i])
		{
			vertex_neighbour[k] = x;
			k++;
		}
	}
	vertex_index[n] = k;
cout<<"Input file reading completed."<<endl;
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
	 int *changed ; // variable for identifying the change in previous iteration
// Memory allocation on device
	cl_mem vertices_device = clCreateBuffer(context, CL_MEM_READ_ONLY, (n+1)*sizeof(int), NULL, &status);
	cl_mem neighbour_device = clCreateBuffer(context, CL_MEM_READ_ONLY, (2*e)*sizeof(int), NULL, &status);
	cl_mem sssp_device = clCreateBuffer(context, CL_MEM_READ_WRITE, n*sizeof(int), NULL, &status);
	cl_mem changed_device = clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_ALLOC_HOST_PTR,sizeof(int), NULL, &status);
// copy data from host to device
	status = clEnqueueWriteBuffer(command_queue, vertices_device, CL_TRUE, 0, (n+1)*sizeof(int), vertex_index, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(command_queue, neighbour_device, CL_TRUE,0, (2*e)*sizeof(int), vertex_neighbour,0,NULL, NULL);
	
// Creating program from kernel sources
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&kernelSource, NULL, &status);
// Build the program
	status = clBuildProgram(program, number_of_devices, devices,NULL, NULL, NULL);
// create OpenCl kernel from the program
	cl_kernel kernel = clCreateKernel(program, "ssspKernel", &status);
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
// Event for synchronization and profiling
cl_event event1,event2,event3;

// Variables for profiling
cl_ulong start1, start2, end1, end2;
double timeInInitialization, timeInKernel=0;
cl_ulong clockPerSec = 1e9;
//Kernel setting
size_t global_size = n;
size_t local_size = 32;
cout<<"Kernel configuration completed."<<endl;


// initialization kernel
status = clEnqueueNDRangeKernel(command_queue, init, 1, NULL, &global_size, &local_size, 0,NULL, &event1);
clWaitForEvents(1, &event1);
cout<<"initialization completed."<<endl;
//Time spent in kernel
status = clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start1, NULL);
status = clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end1, NULL);
timeInInitialization = (double)(end1-start1);
//map the 'changed' variable on host with 'changed_device' variable on device
changed = (int *)clEnqueueMapBuffer(command_queue, changed_device, CL_TRUE, CL_MAP_READ|CL_MAP_WRITE, 0,sizeof(int),0,NULL,NULL,&status);

//Launch sssp kernel in do while loop
int i=0;
do{
	*changed = 0;
	status = clEnqueueNDRangeKernel(command_queue, kernel, 1,NULL, &global_size, &local_size , 0, NULL, &event3);
	clWaitForEvents(1, &event3);
	status = clGetEventProfilingInfo(event3, CL_PROFILING_COMMAND_START,sizeof(cl_ulong), &start2, NULL);
	status = clGetEventProfilingInfo(event3, CL_PROFILING_COMMAND_END,sizeof(cl_ulong), &end2, NULL);
	timeInKernel += (double)(end2- start2);

	i++;
}while(*changed && i<=n);
// unmap the mapped memory
clEnqueueUnmapMemObject(command_queue, changed_device, changed, 0, NULL, NULL);


cout<<"number of times kernel called = "<<i<<endl;
cout<<"Time spent in initialization = "<<timeInInitialization<<" nanosec."<<endl;
cout<<"Time spent in kernel = "<<timeInKernel<<" nanosec"<<endl;
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


// writing the result in output file
for(int i=0;i<n;i++)
{
	fprintf(outputfp, "%d  %d\n",i, sssp[i] );
}
// lets print shortest distance to first 10 nodes
cout<<"Shortest path distance from "<<source<<" to other vertices."<<endl;
for(int i=0;i<min(n,10); i++)
{
	cout<<"vertex:"<<i<<" ,shortest distance:"<<sssp[i]<<endl;
}
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
free(vertex_index);
free(vertex_neighbour);
//free(changed);
free(sssp);
free(devices);
free(platforms);
fclose(inputfp);
fclose(outputfp);
cout<<"Done!"<<endl;
return 0;
}