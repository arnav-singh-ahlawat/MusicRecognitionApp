#pragma once
#include <QObject>
#include <QAudioSource>
#include <QIODevice>
#include <vector>
#include <cstdint>

/**
 * @class AudioCapture
 * @brief Handles microphone recording into a raw PCM16 buffer.
 *
 * Uses Qt Multimedia's QAudioSource to capture audio input.
 * Captured samples are stored in `m_samples` as signed 16-bit integers.
 *
 * Typical usage:
 *   AudioCapture cap;
 *   connect(&cap, &AudioCapture::finished, ...);
 *   cap.start(10);  // record for 10 seconds
 */
class AudioCapture : public QObject {
    Q_OBJECT
public:
    explicit AudioCapture(QObject* parent=nullptr);

    /// Start capturing audio for a fixed duration (seconds)
    void start(int seconds=10);

    /// Stop capturing immediately
    void stop();

    /// Access recorded samples (PCM16, mono)
    const std::vector<int16_t>& samples() const { return m_samples; }

    /// Recording sample rate (Hz)
    int sampleRate() const { return m_sampleRate; }

    signals:
        /// Emitted after recording stops (either by timeout or manual stop)
        void finished();

private:
    int m_sampleRate = 44100; ///< Fixed sample rate (Hz)

    std::unique_ptr<QAudioSource> m_source; ///< Audio input source
    QScopedPointer<QIODevice> m_dev;        ///< Custom device for buffering samples
    std::vector<int16_t> m_samples;         ///< Recorded PCM buffer
};