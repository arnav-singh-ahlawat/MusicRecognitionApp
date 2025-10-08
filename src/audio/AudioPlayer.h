#pragma once
#include <QObject>
#include <QAudioFormat>
#include <QAudioSink>
#include <QIODevice>
#include <vector>
#include <cstdint>

/**
 * @class AudioPlayer
 * @brief Simple wrapper around QAudioSink for PCM playback.
 *
 * Accepts raw PCM16 mono audio data via setBuffer() and plays it
 * through the system's default audio output device.
 *
 * Typical usage:
 *   AudioPlayer player;
 *   player.setBuffer(samples, 44100);
 *   player.play();
 */
class AudioPlayer : public QObject {
    Q_OBJECT
public:
    explicit AudioPlayer(QObject* parent=nullptr);

    /// Load PCM buffer into player (16-bit mono, with given sample rate)
    void setBuffer(const std::vector<int16_t>& samples, int sampleRate);

public slots:
    void play(); ///< Start playback
    void stop(); ///< Stop playback

private:
    QAudioFormat m_format;                   ///< Current audio format
    std::unique_ptr<QAudioSink> m_sink;      ///< Audio output backend
    QByteArray m_pcm;                        ///< Raw PCM data
    QScopedPointer<QIODevice> m_dev;         ///< Buffer device feeding QAudioSink
};