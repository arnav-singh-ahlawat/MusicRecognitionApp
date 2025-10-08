#pragma once
#include <vector>
#include <cstdint>

/**
 * @class Fingerprint
 * @brief Computes audio fingerprints from PCM16 samples.
 *
 * Pipeline:
 *   1. Split signal into overlapping frames with Hann window.
 *   2. Run FFT on each frame and compute magnitude spectrum.
 *   3. Select strongest spectral peaks per frame.
 *   4. Pair anchor peaks with future peaks (target zone).
 *   5. Encode each (f1, f2, Δt) tuple into a 32-bit hash.
 *
 * Output:
 *   Vector of (hash, offset_ms), where offset_ms is the time (ms)
 *   of the anchor peak in the audio stream.
 *
 * These fingerprints are robust to noise and time shifts,
 * enabling fast lookup and matching in a database.
 */
class Fingerprint {
public:
    /// Compute fingerprints for a PCM16 mono signal
    /// @param pcm Raw audio samples
    /// @param sampleRate Sampling rate (Hz)
    /// @return Vector of (hash, offset_ms)
    static std::vector<std::pair<uint32_t,int>> compute(const std::vector<int16_t>& pcm,
                                                        int sampleRate);

private:
    /// Pack frequency pair + time delta into a 32-bit hash
    static uint32_t hashPair(int f1, int f2, int dt);
};