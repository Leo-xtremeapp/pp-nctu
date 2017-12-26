#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

cl_program load_program(cl_context contect, const char *file_name) {
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
  cl_program program = clCreateProgramWithSource(contect, 1, &source, 0, 0);

  if (program == 0 || clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS)
    return 0;

  infile.close();

  return program;
}

int main(int argc, char const *argv[]) {
  // var
  size_t max_items;
  size_t max_work[3];

  cl_int err;
  cl_uint num;

  cl_device_id device_id;
  cl_platform_id platform;

  unsigned int rows;
  unsigned int cols = 3;
  unsigned int buffer;
  unsigned int *image = NULL;

  ifstream infile("input", ios_base::in);
  // ofstream outfile()

  // load input
  infile >> buffer;
  rows = buffer / 3;

  image = new unsigned int[rows];
  for (unsigned int i = 0; i < rows * cols && (infile >> buffer); i++) {
    image[i] = buffer;
    cout << buffer << endl;
  }

  clGetPlatformIDs(1, &platform, NULL);

  err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
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

  cout << max_items << endl;




	return 0;
}
