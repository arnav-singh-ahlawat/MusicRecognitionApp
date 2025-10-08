#pragma once
#include <complex>
#include <vector>

/**
 * @namespace miniFFT
 * @brief Minimal radix-2 FFT implementation with windowing support.
 *
 * Provides:
 *  - In-place Cooley–Tukey FFT (complex input/output).
 *  - Hann window generation (commonly used before FFT to reduce spectral leakage).
 *
 * This implementation is intentionally simple and educational,
 * using std::complex<double> for clarity. Optimizations such as
 * plan reuse, float precision, or SIMD can be added later.
 */
namespace miniFFT {
    using cpx = std::complex<double>;

    /// In-place radix-2 Cooley–Tukey FFT
    /// Input: vector of complex samples (length must be power of 2)
    /// Output: frequency-domain representation (complex spectrum)
    void fft(std::vector<cpx>& a);

    /// Generate Hann window coefficients for length-N buffer
    /// Output: w[i] = 0.5 * (1 - cos(2πi/(N-1)))
    void hannWindow(std::vector<double>& w);
}