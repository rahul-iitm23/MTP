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
"	int row = get_global_id(0);								\n"
"	int col = get_global_id(1);								\n"
"	int sum = 0;											\n"
"	for(int i=0;i<q;i++)									\n"
"	{														\n"
"		sum += A[row*q + i] * B[i*r + col];					\n"
"	}														\n"
"	C[row*r + col] = sum;									\n"
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

	for(int i=0;i<p*q;i++)
	{
		fscanf(inputfp, "%d", A+i);

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

	// Variable for calculating running time
	clock_t start, end;
	double run_time;
	start = clock();


	// Creating context
	cl_context context;
	context = clCreateContext(NULL, number_of_devices, devices, NULL, NULL, &status);
	// Create command queue
	cl_command_queue command_queue ;
	command_queue = clCreateCommandQueue(context, devices[0], 0 , &status);
	// Allocate memory buffer on device
	cl_mem d_A = clCreateBuffer(context, CL_MEM_READ_ONLY, p*q*sizeof(int), NULL, &status);
	cl_mem d_B = clCreateBuffer(context, CL_MEM_READ_ONLY, q*r*sizeof(int), NULL, &status);
	cl_mem d_C = clCreateBuffer(context, CL_MEM_READ_ONLY,p*r*sizeof(int), NULL, &status);

	// Copy data from host to device
	status = clEnqueueWriteBuffer(command_queue, d_A, CL_TRUE, 0, p*q*sizeof(int), A,0, NULL , NULL);
	status = clEnqueueWriteBuffer(command_queue, d_B, CL_TRUE, 0, q*r*sizeof(int), B, 0, NULL, NULL);
	// Creating program from kernel sources
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&matKernel, NULL, &status);
	// Build the program
	status = clBuildProgram(program, number_of_devices, devices,NULL, NULL, NULL);

	// Create the OpenCL kernel
	cl_kernel kernel = clCreateKernel(program, "matrixMult", &status);

	// set arguments for kernel
	status =clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_A);
	status =clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_B);
	status =clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&d_C);
	status =clSetKernelArg(kernel, 3, sizeof(int), (void *)&p);
	status =clSetKernelArg(kernel, 4, sizeof(int), (void *)&q);
	status =clSetKernelArg(kernel, 5, sizeof(int), (void *)&r);
	// Execute the openCL kernel

	const size_t global_size[2] = {(size_t)p,(size_t)r};
	const size_t local_size[2] = {16, 16};
	status = clEnqueueNDRangeKernel(command_queue, kernel , 2, NULL, global_size, local_size, 0, NULL, NULL);
	// reading the result
	status =clEnqueueReadBuffer(command_queue, d_C, CL_TRUE, 0, p*r*sizeof(int), C, 0, NULL, NULL);

	
	// Clean up and wait for all the comands to complete.
	status = clFlush(command_queue);
	status = clFinish(command_queue);


	end = clock();
	run_time = (double)(end-start)/CLOCKS_PER_SEC;
	printf("\nRunning time of kernel = %lf\n", run_time);


	// write the result generated using kernel in output file;
	for(int i=0;i<p;i++)
	{
		for(int j=0;j<r;j++)
		{
			fprintf(outputfp, "%d ", C[i*r +j]);
		}
		fprintf(outputfp, "\n" );
	}

// Finally release all OpenCL allocated objects and host buffers.
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