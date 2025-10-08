#include "AudioCapture.h"
#include <QMediaDevices>
#include <QTimer>
#include <QBuffer>

/// Custom QIODevice that writes incoming PCM16 samples into a vector
class CaptureDevice : public QIODevice {
public:
    explicit CaptureDevice(std::vector<int16_t>& out)
        : QIODevice(), m_out(out) {
        open(QIODevice::WriteOnly);
    }

    /// Called when QAudioSource provides new audio data
    qint64 writeData(const char* data, qint64 len) override {
        // Interpret incoming bytes as signed 16-bit samples
        auto n = len / 2;
        const int16_t* p = reinterpret_cast<const int16_t*>(data);
        m_out.insert(m_out.end(), p, p + n);
        return len;
    }

    /// Not used (capture-only device)
    qint64 readData(char*, qint64) override { return -1; }

private:
    std::vector<int16_t>& m_out; ///< Reference to external buffer
};

AudioCapture::AudioCapture(QObject* parent) : QObject(parent) {}

/// Begin recording audio for `seconds` duration
void AudioCapture::start(int seconds) {
    m_samples.clear();

    // Configure mono 16-bit PCM format
    QAudioFormat fmt;
    fmt.setSampleRate(m_sampleRate);
    fmt.setChannelCount(1);
    fmt.setSampleFormat(QAudioFormat::Int16);

    // Select default input device
    auto devInfo = QMediaDevices::defaultAudioInput();

    // Create audio source and capture device
    m_source.reset(new QAudioSource(devInfo, fmt));
    m_dev.reset(new CaptureDevice(m_samples));

    // Start recording into CaptureDevice
    m_source->start(m_dev.data());

    // Stop after the requested duration
    QTimer::singleShot(seconds * 1000, this, [this] {
        stop();
        emit finished();
    });
}

/// Stop recording (if active)
void AudioCapture::stop() {
    if (m_source) { m_source->stop(); m_source.reset(); }
    if (m_dev)    { m_dev->close(); m_dev.reset(); }
}