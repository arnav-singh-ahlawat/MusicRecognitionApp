#include "Fingerprint.h"
#include "FFT.h"
#include <algorithm>
#include <cmath>

#if USE_OPENCL
#include "OpenCLAccel.h"     // for GPU-based acceleration
static OpenCLAccel g_opencl; // Global/shared OpenCL accelerator
#endif

// ---- Parameters tuned for 44.1 kHz audio ----
static constexpr int WINDOW_SIZE   = 2048;   // ~46 ms
static constexpr int HOP_SIZE      = 1024;   // 50% overlap
static constexpr int TOP_PEAKS     = 5;      // strongest peaks per frame
static constexpr int FANOUT        = 5;      // max target pairs per anchor
static constexpr int TARGET_DT_MIN = 1;      // frames (min lookahead)
static constexpr int TARGET_DT_MAX = 20;     // frames (~1s lookahead)

// Map FFT bin index to a coarse frequency band (logarithmic-ish)
static int freqToBand(int bin, int fftSize, int sr) {
    double freq = double(bin) * sr / fftSize;
    if (freq < 200) return 0;
    if (freq < 400) return 1;
    if (freq < 800) return 2;
    if (freq < 1600) return 3;
    if (freq < 3200) return 4;
    if (freq < 6400) return 5;
    return 6;
}

/// Encode two frequency bins and time delta into a 32-bit hash
uint32_t Fingerprint::hashPair(int f1, int f2, int dt) {
    // Pack bits: f1(10) | f2(10) | dt(12)
    if (f1 > 1023) f1 = 1023;
    if (f2 > 1023) f2 = 1023;
    if (dt > 4095) dt = 4095;

    return (uint32_t)((f1 & 0x3FF) << 22) |
           ((f2 & 0x3FF) << 12) |
           (dt & 0xFFF);
}

/// Compute audio fingerprints
std::vector<std::pair<uint32_t,int>> Fingerprint::compute(const std::vector<int16_t>& pcm, int sr) {
    const int N = (int)pcm.size();
    if (N < WINDOW_SIZE) return {};

    // Precompute Hann window
    std::vector<double> window(WINDOW_SIZE);
    miniFFT::hannWindow(window);

    std::vector<std::pair<int,int>> peaksPerFrame; // (bin, frameIdx)
    peaksPerFrame.reserve(N / HOP_SIZE * TOP_PEAKS);

    std::vector<miniFFT::cpx> buf(WINDOW_SIZE);
    std::vector<double> mag(WINDOW_SIZE/2);

    int frameIdx = 0;
    for (int start = 0; start + WINDOW_SIZE <= N; start += HOP_SIZE, ++frameIdx) {
        // ---- Windowed frame ----
        for (int i = 0; i < WINDOW_SIZE; i++) {
            double s = pcm[start+i] / 32768.0; // normalize
            buf[i] = miniFFT::cpx(s * window[i], 0.0);
        }

        // ---- FFT ----
        miniFFT::fft(buf);

        // ---- Magnitude/power spectrum (GPU first, CPU fallback) ----
        bool usedGPU = false;
        #if USE_OPENCL
        if (g_opencl.ok()) {
            // Convert FFT buffer to interleaved float2
            std::vector<float> interleaved;
            interleaved.reserve(WINDOW_SIZE*2);
            for (auto& c : buf) {
                interleaved.push_back(static_cast<float>(c.real()));
                interleaved.push_back(static_cast<float>(c.imag()));
            }

            std::vector<float> gpuMag;
            if (g_opencl.magnitudeBatch(interleaved, 1, WINDOW_SIZE, gpuMag)) {
                for (int k = 0; k < WINDOW_SIZE/2; k++) {
                    mag[k] = gpuMag[k]; // GPU-accelerated power spectrum
                }
                usedGPU = true;
            }
        }
        #endif

        if (!usedGPU) {
            // CPU fallback
            for (int k = 0; k < WINDOW_SIZE/2; k++) {
                mag[k] = std::norm(buf[k]); // CPU-accelerated power spectrum
            }
        }

        // ---- Peak selection ----
        std::vector<std::pair<double,int>> bins;
        bins.reserve(WINDOW_SIZE/2);
        for (int k = 5; k < WINDOW_SIZE/2; k++) {
            bins.emplace_back(mag[k], k);
        }

        // Keep top-N strongest bins
        std::nth_element(bins.begin(),
                         bins.begin() + std::min(TOP_PEAKS, (int)bins.size()),
                         bins.end(),
                         [](auto& a, auto& b){ return a.first > b.first; });

        int take = std::min(TOP_PEAKS, (int)bins.size());
        for (int i = 0; i < take; i++) {
            peaksPerFrame.emplace_back(bins[i].second, frameIdx);
        }
    }

    // ---- Build hash pairs ----
    std::vector<std::pair<uint32_t,int>> out;
    out.reserve(peaksPerFrame.size() * 2);

    // Index peaks by frame
    int totalFrames = frameIdx;
    std::vector<std::vector<int>> peaksInFrame(totalFrames);
    for (auto& p : peaksPerFrame) {
        peaksInFrame[p.second].push_back(p.first);
    }

    // Anchor peak -> pair with targets in future frames
    for (int a = 0; a < totalFrames; ++a) {
        auto& A = peaksInFrame[a];
        if (A.empty()) continue;

        int targetsAdded = 0;
        for (int t = a + TARGET_DT_MIN; t <= std::min(a + TARGET_DT_MAX, totalFrames-1); ++t) {
            auto& T = peaksInFrame[t];
            for (int f1 : A) {
                for (int f2 : T) {
                    // Reduce dimensionality with banding
                    int b1 = freqToBand(f1, WINDOW_SIZE, sr) * 128 + (f1 % 128);
                    int b2 = freqToBand(f2, WINDOW_SIZE, sr) * 128 + (f2 % 128);

                    uint32_t h = hashPair(b1, b2, t - a);

                    // Anchor time in ms
                    int offset_ms = int((a * HOP_SIZE * 1000.0) / sr);

                    out.emplace_back(h, offset_ms);

                    if (++targetsAdded >= FANOUT) break;
                }
                if (targetsAdded >= FANOUT) break;
            }
            if (targetsAdded >= FANOUT) break;
        }
    }

    return out;
}