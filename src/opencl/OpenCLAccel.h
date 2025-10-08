#pragma once

// Compile-time toggle: CMake sets USE_OPENCL=0 by default.
// When enabled, this class can offload parts of DSP to the GPU.
#ifndef USE_OPENCL
#define USE_OPENCL 0
#endif

#if USE_OPENCL
#include <CL/cl.h>
#include <vector>

/**
 * @class OpenCLAccel
 * @brief GPU-accelerated helper for DSP workloads using OpenCL.
 *
 * This class attempts to:
 *   - Find a GPU device
 *   - Create an OpenCL context and command queue
 *   - Load and build kernels from "kernels/fingerprint.cl"
 *
 * Current status:
 *   - Initialization works (device, context, program).
 *   - Provides magnitude computation via `magnitudeBatch()`,
 *     which computes power spectra (|x|^2) from complex input frames.
 */
class OpenCLAccel {
public:
    OpenCLAccel();
    ~OpenCLAccel();

    /// Check if initialization succeeded
    bool ok() const { return m_ok; }

    /// Example API: batch magnitude computation (stub)
    bool magnitudeBatch(const std::vector<float>& frames,
                        int frameCount,
                        int frameSize,
                        std::vector<float>& outPower);

private:
    bool m_ok = false;
    cl_context m_ctx = nullptr;
    cl_command_queue m_q = nullptr;
    cl_program m_prog = nullptr;
    cl_device_id m_dev = nullptr;
};

#else
/**
 * @class OpenCLAccel
 * @brief Disabled stub when USE_OPENCL=0 (no GPU acceleration).
 */
class OpenCLAccel {
public:
    bool ok() const { return false; }
};
#endif