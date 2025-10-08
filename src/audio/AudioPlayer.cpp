#include "AudioPlayer.h"
#include <QMediaDevices>
#include <QBuffer>

AudioPlayer::AudioPlayer(QObject* parent) : QObject(parent) {
    // Default to 44.1 kHz, mono, 16-bit PCM
    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);
}

/// Store audio samples in an internal QByteArray for playback
void AudioPlayer::setBuffer(const std::vector<int16_t>& samples, int sr) {
    m_format.setSampleRate(sr);
    m_pcm = QByteArray(reinterpret_cast<const char*>(samples.data()),
                       int(samples.size() * sizeof(int16_t)));
}

/// Play the loaded buffer using system's default audio output
void AudioPlayer::play() {
    auto devInfo = QMediaDevices::defaultAudioOutput();

    // Create audio sink with chosen format
    m_sink.reset(new QAudioSink(devInfo, m_format));

    // Wrap QByteArray in a QBuffer so QAudioSink can read it
    auto* buf = new QBuffer(&m_pcm);
    buf->open(QIODevice::ReadOnly);

    m_dev.reset(buf);

    // Start playback
    m_sink->start(m_dev.data());
}

/// Stop playback (if active)
void AudioPlayer::stop() {
    if (m_sink) { m_sink->stop(); m_sink.reset(); }
    if (m_dev)  { m_dev->close(); m_dev.reset(); }
}