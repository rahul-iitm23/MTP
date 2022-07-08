#include<iostream>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<CL/cl.h>
using namespace std;
const char * matKernel =
"__kernel void matrixMult(__global int *A, __global int *B,\n"
" __global int *C, const int p, const int q, const int r)	\n"
"{															\n"
"	int row = get_global_id(0); 							\n"
"	int col = get_global_id(1); 							\n"
"	int b = B[row*r + col];									\n"
"	for(int k=0;k<p;k++)									\n"
"	{														\n"
"	 atomic_add(&C[ k*r + col] , A[row*p+k] * b );			\n"
"	}														\n"									
"}															\n"
"__kernel void initialize(__global int *C, int r)           \n"
"{															\n"
"	C[get_global_id(0)*r + get_global_id(1)] = 0;			\n"
"}															\n";




int main(int argc, char *arg[])
{
	// file pointers for reading and writing to file.
	FILE * inputfp = fopen(arg[1],"r");
	FILE * outputfp = fopen(arg[2], "w");

	// variables for size of matrices
	int p,q,r;
	fscanf(inputfp, "%d %d %d", &p, &q, &r);

	// Creating 3 pointers, two for input matrix and one for output matrix
	int *A;
	int *B;
	int *C;

	// Allocating space using malloc
	A = (int * )malloc(p*q*sizeof(int ));
	B = (int *)malloc(q*r*sizeof(int));
	C = (int *)malloc(p*r*sizeof(int));
	
	// reading input matrices from input file
	// Storing A as A^T
	for(int i=0;i<p;i++)
	{
		for(int j=0;j<q;j++)
		{
			fscanf(inputfp, "%d ", &A[j*p + i ]);
		}
	}  

	for(int i=0;i<q*r;i++)
	{
		fscanf(inputfp, "%d", B+i);
	}

	// Getting platform
	
	cl_platform_id *platforms=NULL;
	cl_uint number_of_platform;
	cl_int status;
	status = clGetPlatformIDs(0, NULL, &number_of_platform);
	platforms = (cl_platform_id *)malloc(number_of_platform* sizeof(cl_platform_id));
	status = clGetPlatformIDs(number_of_platform, platforms, NULL);

	// Getting devices from the platform

	cl_device_id * devices=NULL;
	cl_uint  number_of_devices;
	status = clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_GPU, 0, NULL, &number_of_devices);
	devices = (cl_device_id *)malloc(number_of_devices *sizeof(cl_device_id));
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, number_of_devices, devices, NULL);

	

	// Creating context
	cl_context context;
	context = clCreateContext(NULL, number_of_devices, devices, NULL, NULL, &status);
	// Create command queue
	cl_command_queue command_queue ;
	command_queue = clCreateCommandQueue(context, devices[0], CL_QUEUE_PROFILING_ENABLE , &status);
	// Allocate memory buffer on device
	cl_mem d_A = clCreateBuffer(context, CL_MEM_READ_ONLY, p*q*sizeof(int), NULL, &status);
	cl_mem d_B = clCreateBuffer(context, CL_MEM_READ_ONLY, q*r*sizeof(int), NULL, &status);
	cl_mem d_C = clCreateBuffer(context, CL_MEM_READ_WRITE,p*r*sizeof(int), NULL, &status);

	// Copy data from host to device
	status = clEnqueueWriteBuffer(command_queue, d_A, CL_TRUE, 0, p*q*sizeof(int), A,0, NULL , NULL);
	status = clEnqueueWriteBuffer(command_queue, d_B, CL_TRUE, 0, q*r*sizeof(int), B, 0, NULL, NULL);
	// Creating program from kernel sources
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&matKernel, NULL, &status);
	// Build the program
	status = clBuildProgram(program, number_of_devices, devices,NULL, NULL, NULL);

	// Create the OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "matrixMult", &status);
	cl_kernel kernel_init = clCreateKernel(program, "initialize", &status);

	// set arguments for kernel
	status =clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_A);
	status =clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_B);
	status =clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&d_C);
	status =clSetKernelArg(kernel, 3, sizeof(int), (void *)&p);
	status =clSetKernelArg(kernel, 4, sizeof(int), (void *)&q);
	status =clSetKernelArg(kernel, 5, sizeof(int), (void *)&r);

	// setting kernel argument for initialization kernel
	status = clSetKernelArg(kernel_init, 0, sizeof(cl_mem), (void *)&d_C);
	status = clSetKernelArg(kernel_init, 1, sizeof(int), (void *)&r);
	// variable for profiling
	cl_event event1, event2;


	// Execute the openCL kernel
	const size_t global_size_init[2] = {(size_t)p, (size_t)r};
	const size_t global_size[2] = {(size_t)q,(size_t)r};
	const size_t local_size[2] = {16, 16};
	status = clEnqueueNDRangeKernel(command_queue, kernel_init, 2, NULL, global_size_init, local_size, 0, NULL, &event1);
	status = clEnqueueNDRangeKernel(command_queue, kernel , 2, NULL, global_size, local_size, 1, &event1, &event2);
	clWaitForEvents(1, &event2);
	//time spent in kernel
	cl_ulong start1, start2, end1, end2;
	cl_ulong x = 1e9;
	status = clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start1, NULL);
	status = clGetEventProfilingInfo(event1, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end1, NULL);
	cout<<"Time spent in initialization kernel = "<<((double)(end1-start1))/x<<" seconds"<<endl;
	status = clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start2, NULL);
	status = clGetEventProfilingInfo(event2, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end2, NULL);
	cout<<"Time spent in matrixMult kernel = "<<((double)(end2-start2))/x<<" seconds"<<endl;
	cout<<"Total time spent in matrix multiplication = "<< ((double)(end2-start2) +(double)(end1-start1))/x<<" seconds"<<endl;


	// reading the result
	status =clEnqueueReadBuffer(command_queue, d_C, CL_TRUE, 0, p*r*sizeof(int), C, 0, NULL, NULL);

	
	// Clean up and wait for all the comands to complete.
	status = clFlush(command_queue);
	status = clFinish(command_queue);

	


 /* // write the result generated using kernel in output file;
	for(int i=0;i<p;i++)
	{
		for(int j=0;j<r;j++)
		{
			fprintf(outputfp, "%d ", C[i*r +j]);
		}
		fprintf(outputfp, "\n" );
	}
	
*/
// Finally release all OpenCL allocated objects and host buffers.
status =clReleaseEvent(event1);
status = clReleaseEvent(event2);
status = clReleaseKernel(kernel);
status = clReleaseProgram(program);
status = clReleaseMemObject(d_A);
status = clReleaseMemObject(d_B);
status = clReleaseMemObject(d_C);
status = clReleaseCommandQueue(command_queue);
status = clReleaseContext(context);
free(A);
free(B);
free(C);
free(platforms);
free(devices);
fclose(inputfp);
fclose(outputfp);
printf("Done!\n");
return 0;
}
