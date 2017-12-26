#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

cl_program load_program(cl_context context, const char *file_name) {
  // var
  size_t length;
  vector<char> data;
  ifstream infile(file_name, ios_base::in);

  infile.seekg(0, ios_base::end);
  length = infile.tellg();
  infile.seekg(0, ios_base::beg);

  data = vector<char>(length + 1);
  infile.read(&data[0], length);
  data[length] = 0;

  const char *source = &data[0];
  cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);

  if (program == 0 || clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS)
    return 0;

  infile.close();

  return program;
}

int main(int argc, char const *argv[]) {
  // var
  size_t max_items;
  size_t max_work[3];
  size_t local_work_size;
  size_t global_work_size;


  cl_int err;
  cl_uint num;

  cl_device_id device_id;
  cl_platform_id platform_id;
  cl_program program;
  cl_kernel kernel;
  cl_command_queue commands;
  cl_context context;

  cl_mem input;
  cl_mem output;

  unsigned int buffer;
  unsigned int total_tasks;
  unsigned int *image = NULL;
  unsigned int task_per_thread;
  unsigned int results[256 * 3];

  ifstream infile("input", ios_base::in);
  ofstream outfile("xxxxxx.out", ios_base::out);

  // load input
  infile >> buffer;
  total_tasks = buffer / 3;

  image = new unsigned int[total_tasks * 3];

  for (unsigned int i = 0; i < total_tasks * 3 && (infile >> buffer); i++) {
    image[i] = buffer;
  }

  clGetPlatformIDs(1, &platform_id, NULL);

  err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
  if (err != CL_SUCCESS) {
    printf("clGetDeviceIDs(): %d\n", err);
    return EXIT_FAILURE;
  }

  err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(max_work), &max_work, NULL);
  if (err != CL_SUCCESS) {
    printf("clGetDeviceInfo(): %d\n", err);
    return EXIT_FAILURE;
  }

  max_items = max_work[0] * max_work[1] * max_work[2];

  // assign jobs
  task_per_thread = total_tasks / max_items + 1;

  context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
  if (!context) {
    printf("clCreateContext(): %d\n", err);
    return EXIT_FAILURE;
  }

  commands = clCreateCommandQueue(context, device_id, 0, &err);
  if (!commands) {
    printf("clCreateCommandQueue(): %d\n", err);
    return EXIT_FAILURE;
  }

  program = load_program(context, "histogram.cl");
  if (!program) {
    printf("load_program(): %d\n", program);
    return EXIT_FAILURE;
  }

  err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    printf("clBuildProgram(): %d\n", err);
    return EXIT_FAILURE;
  }

  kernel = clCreateKernel(program, "histogram", &err);
  if (!kernel || err != CL_SUCCESS) {
    printf("clCreateKernel(): %d, %d\n", kernel, err);
    return EXIT_FAILURE;
  }

  input = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(unsigned int) * total_tasks * 3, NULL, NULL);
  output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(unsigned int) * 256 * 3, NULL, NULL);
  if (!input || !output) {
    printf("clCreateBuffer()\n");
    return EXIT_FAILURE;
  }

  err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(unsigned int) * total_tasks * 3, image, 0, NULL, NULL);
  if (err != CL_SUCCESS) {
    printf("clEnqueueWriteBuffer(): %d\n", err);
    return EXIT_FAILURE;
  }

  err = 0;
  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
  err = clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
  err = clSetKernelArg(kernel, 2, sizeof(unsigned int), &total_tasks);
  err = clSetKernelArg(kernel, 3, sizeof(unsigned int), &task_per_thread);
  if (err != CL_SUCCESS) {
    printf("clSetKernelArg(): %d\n", err);
    return EXIT_FAILURE;
  }

  err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local_work_size), &local_work_size, NULL);
  if (err != CL_SUCCESS) {
    printf("clGetKernelWorkGroupInfo(): %d\n", err);
    return EXIT_FAILURE;
  }

  global_work_size = max_items;
  err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global_work_size, &local_work_size, 0, NULL, NULL);
  if (err != CL_SUCCESS) {
    printf("clEnqueueNDRangeKernel(): %d\n", err);
    return EXIT_FAILURE;
  }

  clFinish(commands);

  err = clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(unsigned int) * 256 * 3, results, 0, NULL, NULL);
  if (err != CL_SUCCESS) {
    printf("clEnqueueReadBuffer(): %d\n", err);

    printf("%d, %d, %d, %d, %d\n", CL_INVALID_COMMAND_QUEUE, CL_INVALID_CONTEXT, CL_INVALID_MEM_OBJECT, CL_INVALID_VALUE, CL_INVALID_EVENT_WAIT_LIST);
    printf("%d, %d, %d, %d, %d\n", CL_MISALIGNED_SUB_BUFFER_OFFSET, CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST, CL_MEM_OBJECT_ALLOCATION_FAILURE, CL_OUT_OF_RESOURCES);
    cout << CL_OUT_OF_HOST_MEMORY << endl;

    return EXIT_FAILURE;
  }

  for (unsigned int i = 0; i < 256 * 3; i++) {
    if (i % 256 == 0 && i != 0)
      outfile << endl;
    outfile << results[i] << " ";
  }

  infile.close();
  outfile.close();

  clReleaseMemObject(input);
  clReleaseMemObject(output);
  clReleaseProgram(program);
  clReleaseKernel(kernel);
  clReleaseCommandQueue(commands);
  clReleaseContext(context);

	return 0;
}
