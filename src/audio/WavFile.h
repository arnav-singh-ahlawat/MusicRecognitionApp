#pragma once
#include <QByteArray>
#include <QString>
#include <vector>
#include <cstdint>

/**
 * @struct WavInfo
 * @brief Metadata extracted from a WAV file.
 *
 * Includes sample rate, number of channels, bit depth, and total frame count.
 */
struct WavInfo {
    int sampleRate = 44100;
    int channels = 1;
    int bitsPerSample = 16;
    int64_t totalFrames = 0;
};

/**
 * @class WavFile
 * @brief Minimal WAV reader/writer for PCM16 audio.
 *
 * Supports:
 *   - Loading uncompressed PCM16 WAV files (mono/stereo).
 *   - Saving mono PCM16 audio (used for captured microphone data).
 */
class WavFile {
public:
    /// Load uncompressed PCM16 WAV into samples + metadata.
    /// Returns false on error (err set if provided).
    static bool loadPcm16(const QString& path,
                          std::vector<int16_t>& samples,
                          WavInfo& info,
                          QString* err=nullptr);

    /// Save mono PCM16 audio into a WAV file.
    static bool saveMonoPcm16(const QString& path,
                              const std::vector<int16_t>& samples,
                              int sampleRate,
                              QString* err=nullptr);
};