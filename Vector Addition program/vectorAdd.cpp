#include<iostream>
#include<stdio.h>
#include<time.h>
#include<stdlib.h>
#include<CL/cl.h>
using namespace std;

const char *vec_add_kernel =
"__kernel       						\n"
"void vectorAddition(__global int *A, \n"
" __global int *B, 					\n"
" __global int *C)					 \n"
"{										 \n"
" //Get the index of the work-item 		 \n"
" int index = get_global_id(0); 		 \n"
" C[index] = A[index] + B[index];		 \n"
"}										 \n";

void cpu_function(int *A, int *B, int n)
{
	int *C = (int *)malloc(n*sizeof(int));
	for(int i=0;i<n;i++)
	{
		C[i] = A[i]+B[i];
	}
	return;
}


int main(int argc, char *arg[])
{
	FILE * inputFileP = fopen(arg[1],"r");
	FILE * outputFileP = fopen(arg[2], "w");

	int n;
	fscanf(inputFileP, "%d", &n);
	// Allocating memory on host
	int *A = (int *)malloc(n*sizeof(int));
	int *B = (int *)malloc(n*sizeof(int));
	int *C = (int *)malloc(n*sizeof(int));
// taking input

	for(int i=0;i<n;i++)
	{
		fscanf(inputFileP, "%d ", &A[i]);
	}
	for(int i=0;i<n;i++)
	{
		fscanf(inputFileP, "%d", &B[i]);
	}
//Get Platform
	cl_platform_id * platforms = NULL;
	cl_uint num_of_platform;
	cl_int status;
	status = clGetPlatformIDs(0, NULL, &num_of_platform);
	platforms = (cl_platform_id *) malloc(num_of_platform * sizeof(cl_platform_id));
	status = clGetPlatformIDs(num_of_platform, platforms, NULL);
// Printing Platform info
	char buffer[1024];
	for(int i=0;i<num_of_platform;i++)
	{
		status = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, sizeof(buffer), buffer, NULL);
		printf("Platform name:%s\n", buffer);
		buffer[0] = '\0';
		status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, sizeof(buffer), buffer, NULL);
		printf("Platform vendor : %s\n", buffer);
		buffer[0] = '\0';
		status = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION , sizeof(buffer), buffer, NULL);
		printf("Platform version : %s\n", buffer);
		buffer[0] = '\0';
	}
	// Get devices from the first platform
	cl_device_id * devices = NULL;
	cl_uint num_of_device;
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 0, NULL, &num_of_device);
	devices = (cl_device_id *)malloc(num_of_device * sizeof(cl_device_id));
	status = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, num_of_device, devices,NULL);
	// print device info
	int compute_units;
	for(int i=0;i<num_of_device;i++)
	{
	
		status = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, sizeof(buffer), buffer, NULL);
		printf("Device name :%s\n",buffer );
		buffer[0] = '\0';
		status = clGetDeviceInfo(devices[i], CL_DEVICE_VENDOR, sizeof(buffer), buffer, NULL);
		printf("Device VENDOR :%s\n",buffer );
		buffer[0] = '\0';
		status = clGetDeviceInfo(devices[i], CL_DEVICE_VERSION, sizeof(buffer), buffer, NULL);
		printf("Device VERSION :%s\n",buffer );
		buffer[0] = '\0';
		status = clGetDeviceInfo(devices[i], CL_DRIVER_VERSION, sizeof(buffer), buffer, NULL);
		printf("DRIVER VERSION :%s\n",buffer );
		buffer[0] = '\0';
		status = clGetDeviceInfo(devices[i], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(int), &compute_units, NULL);
		printf("CL_DEVICE_MAX_COMPUTE_UNITS :%d\n",compute_units );

	}
// Variables for time
clock_t start, end;
double time_taken;
// start the clock for vector addition kernel
start = clock();
// Creating context
	cl_context context;
	context = clCreateContext(NULL, num_of_device, devices, NULL, NULL, &status);
// Create command queue
	cl_command_queue command_queue ;
	command_queue = clCreateCommandQueue(context, devices[0], 0 , &status);

// create memory buffer on device for each vector
	cl_mem d_A = clCreateBuffer(context, CL_MEM_READ_ONLY, n*sizeof(int), NULL, &status);
	cl_mem d_B = clCreateBuffer(context, CL_MEM_READ_ONLY, n*sizeof(int), NULL, &status);
	cl_mem d_C = clCreateBuffer(context, CL_MEM_WRITE_ONLY, n*sizeof(int), NULL, &status);
//Copy the vector A and B  to device
	status = clEnqueueWriteBuffer(command_queue, d_A, CL_TRUE, 0, n*sizeof(int), A, 0, NULL, NULL);
	status = clEnqueueWriteBuffer(command_queue , d_B, CL_TRUE, 0, n*sizeof(int), B, 0, NULL, NULL);
//Creating  program from kernel source 
	cl_program program = clCreateProgramWithSource(context, 1, (const char **)&vec_add_kernel, NULL, &status);
// Build the program
	status = clBuildProgram(program, num_of_device, devices, NULL, NULL, NULL);
// create the openCL kernel
	cl_kernel kernel = clCreateKernel(program, "vectorAddition", &status);

// set argument to kernel 
	status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&d_A);
	status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *)&d_B);
	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *)&d_C);
// Execute the openCL kernel
	size_t global_size = n;
	size_t local_size = 64;
	status = clEnqueueNDRangeKernel(command_queue, kernel , 1, NULL, &global_size, &local_size, 0, NULL, NULL);
// Read the C buffer device to host memory transfer
	status = clEnqueueReadBuffer(command_queue, d_C, CL_TRUE, 0, n*sizeof(int), C, 0, NULL, NULL);

// Clean up and wait for all the comands to complete.
	status = clFlush(command_queue);
	status = clFinish(command_queue);
// end the clock for vector addition kernel
end = clock();
time_taken = (double) (end-start)/CLOCKS_PER_SEC;
printf("Time taken by vectorAddition kernel = %0.6lf sec\n", time_taken); 

// Let see how much time cpu takes to do same computation(using the same variables for time)
start = clock();
cpu_function(A, B,n);
end = clock();
time_taken = (double)(end-start)/CLOCKS_PER_SEC;
printf("Time taken by cpu to do the same coputation: %0.6lf sec\n",time_taken );

// Storing result in output file
	for(int i=0;i<n;i++)
	{
		fprintf(outputFileP, "%d ", C[i]);
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
fclose(inputFileP);
fclose(outputFileP);
printf("\nDone!\n");
return 0;

}