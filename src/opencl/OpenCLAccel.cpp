#include "OpenCLAccel.h"

#if USE_OPENCL
#include <fstream>
#include <string>

// Utility: load OpenCL kernel source from file
static std::string loadKernel(const char* path) {
    std::ifstream ifs(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(ifs)),
                       std::istreambuf_iterator<char>());
}

OpenCLAccel::OpenCLAccel() {
    cl_uint nplat = 0;
    clGetPlatformIDs(0, nullptr, &nplat);
    if (!nplat) return;

    std::vector<cl_platform_id> plats(nplat);
    clGetPlatformIDs(nplat, plats.data(), nullptr);

    cl_int err = 0;
    cl_device_id dev = nullptr;

    // Prefer GPU devices
    for (auto p : plats) {
        cl_uint ndev = 0;
        clGetDeviceIDs(p, CL_DEVICE_TYPE_GPU, 1, &dev, &ndev);
        if (ndev) { m_dev = dev; break; }
    }
    if (!m_dev) return;

    // Create context + queue
    m_ctx = clCreateContext(nullptr, 1, &m_dev, nullptr, nullptr, &err);
    if (err) return;

    const cl_queue_properties props[] = { CL_QUEUE_PROPERTIES, 0, 0 };
    m_q = clCreateCommandQueueWithProperties(m_ctx, m_dev, props, &err);
    if (err) return;

    // Load and build program
    std::string src = loadKernel("kernels/fingerprint.cl");
    const char* s = src.c_str();
    size_t len = src.size();

    m_prog = clCreateProgramWithSource(m_ctx, 1, &s, &len, &err);
    if (err) return;

    err = clBuildProgram(m_prog, 1, &m_dev, "", nullptr, nullptr);
    if (err) return;

    m_ok = true;
}

OpenCLAccel::~OpenCLAccel() {
    if (m_prog) clReleaseProgram(m_prog);
    if (m_q) clReleaseCommandQueue(m_q);
    if (m_ctx) clReleaseContext(m_ctx);
}

bool OpenCLAccel::magnitudeBatch(const std::vector<float>& frames,
                                 int frameCount,
                                 int frameSize,
                                 std::vector<float>& outPower) {
#if USE_OPENCL
    if (!m_ok) return false;

    cl_int err = 0;

    // Create kernel
    cl_kernel kernel = clCreateKernel(m_prog, "mag_kernel", &err);
    if (err != CL_SUCCESS) return false;

    size_t totalElems = size_t(frameCount) * frameSize;
    size_t inBytes = totalElems * sizeof(cl_float2);  // input is float2 (complex)
    size_t outBytes = totalElems * sizeof(cl_float);

    // Input: reinterpret frames as complex numbers
    // Assumes frames is laid out [frameCount][frameSize*2] as interleaved float2
    const cl_float2* inPtr = reinterpret_cast<const cl_float2*>(frames.data());

    // Allocate device buffers
    cl_mem bufIn  = clCreateBuffer(m_ctx, CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR,
                                   inBytes, (void*)inPtr, &err);
    if (err != CL_SUCCESS) { clReleaseKernel(kernel); return false; }

    cl_mem bufOut = clCreateBuffer(m_ctx, CL_MEM_WRITE_ONLY,
                                   outBytes, nullptr, &err);
    if (err != CL_SUCCESS) { clReleaseMemObject(bufIn); clReleaseKernel(kernel); return false; }

    // Set kernel args
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufIn);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufOut);
    err |= clSetKernelArg(kernel, 2, sizeof(int),    &frameSize);
    if (err != CL_SUCCESS) {
        clReleaseMemObject(bufIn); clReleaseMemObject(bufOut); clReleaseKernel(kernel);
        return false;
    }

    // Launch kernel (1 work-item per FFT bin)
    size_t globalSize = totalElems;
    err = clEnqueueNDRangeKernel(m_q, kernel, 1, nullptr, &globalSize, nullptr, 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        clReleaseMemObject(bufIn); clReleaseMemObject(bufOut); clReleaseKernel(kernel);
        return false;
    }

    // Read results back
    outPower.resize(totalElems);
    err = clEnqueueReadBuffer(m_q, bufOut, CL_TRUE, 0, outBytes, outPower.data(), 0, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        clReleaseMemObject(bufIn); clReleaseMemObject(bufOut); clReleaseKernel(kernel);
        return false;
    }

    // Cleanup
    clReleaseMemObject(bufIn);
    clReleaseMemObject(bufOut);
    clReleaseKernel(kernel);

    return true;
#else
    (void)frames; (void)frameCount; (void)frameSize; (void)outPower;
    return false;
#endif
}
#endif