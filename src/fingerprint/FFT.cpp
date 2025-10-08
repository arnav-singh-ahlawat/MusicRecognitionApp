#include "FFT.h"
#include <cmath>

using namespace miniFFT;

/// Iterative radix-2 FFT (Cooley–Tukey)
void miniFFT::fft(std::vector<cpx>& a) {
    const size_t n = a.size();
    if (n <= 1) return;

    // ---- Bit-reversal permutation ----
    // Reorder elements so butterflies access contiguous memory
    for (size_t i = 1, j = 0; i < n; i++) {
        size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }

    // ---- Iterative FFT butterflies ----
    for (size_t len = 2; len <= n; len <<= 1) {
        double ang = 2 * M_PI / len;
        cpx wlen(std::cos(ang), std::sin(ang)); // primitive root of unity

        for (size_t i = 0; i < n; i += len) {
            cpx w(1, 0);
            for (size_t j = 0; j < len / 2; ++j) {
                cpx u = a[i + j];
                cpx v = a[i + j + len/2] * w;

                // Butterfly operations
                a[i + j]         = u + v;
                a[i + j + len/2] = u - v;

                w *= wlen;
            }
        }
    }
}

/// Compute Hann window coefficients
/// Used to reduce spectral leakage before FFT
void miniFFT::hannWindow(std::vector<double>& w) {
    const size_t n = w.size();
    for (size_t i = 0; i < n; i++) {
        w[i] = 0.5 * (1.0 - std::cos(2 * M_PI * i / (n - 1)));
    }
}