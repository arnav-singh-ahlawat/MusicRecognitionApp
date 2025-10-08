#include "WavFile.h"
#include <QFile>
#include <QtEndian>
#include <iostream>

// Helper to read a WAV chunk header (id + size)
static bool readChunkHeader(QFile& f, QByteArray& id, quint32& size) {
    id = f.read(4);
    if (id.size() != 4) return false;
    if (f.read(reinterpret_cast<char*>(&size), 4) != 4) return false;
    return true;
}

/// Load PCM16 WAV file into memory
bool WavFile::loadPcm16(const QString& path,
                        std::vector<int16_t>& out,
                        WavInfo& info,
                        QString* err) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) {
        if (err) *err = "Cannot open file";
        return false;
    }

    // --- RIFF/WAVE header ---
    char riff[4]; quint32 riffSize; char wave[4];
    if (f.read(riff, 4) != 4 ||
        f.read(reinterpret_cast<char*>(&riffSize), 4) != 4 ||
        f.read(wave, 4) != 4) {
        if (err) *err = "Invalid RIFF header";
        return false;
    }
    if (QByteArray(riff, 4) != "RIFF" || QByteArray(wave, 4) != "WAVE") {
        if (err) *err = "Not a WAV file";
        return false;
    }

    // --- Parse chunks ---
    bool hasFmt = false;
    quint32 dataSize = 0;
    qint64 dataPos = -1;

    quint16 audioFormat = 0;
    quint16 numChannels = 0;
    quint32 sampleRate = 0;
    quint16 bitsPerSample = 0;

    while (!f.atEnd()) {
        QByteArray id; quint32 sz = 0;
        if (!readChunkHeader(f, id, sz)) break;

        if (id == "fmt ") {
            // Parse format chunk
            if (sz < 16) {
                if (err) *err = "fmt chunk too small";
                return false;
            }

            f.read(reinterpret_cast<char*>(&audioFormat), 2);
            f.read(reinterpret_cast<char*>(&numChannels), 2);
            f.read(reinterpret_cast<char*>(&sampleRate), 4);

            quint32 byteRate; quint16 blockAlign;
            f.read(reinterpret_cast<char*>(&byteRate), 4);
            f.read(reinterpret_cast<char*>(&blockAlign), 2);
            f.read(reinterpret_cast<char*>(&bitsPerSample), 2);

            // Skip any extra format fields
            if (sz > 16) {
                f.seek(f.pos() + (sz - 16));
            }

            hasFmt = true;
        }
        else if (id == "data") {
            // Data chunk: remember position & size
            dataPos = f.pos();
            dataSize = sz;
            f.seek(f.pos() + sz);
        }
        else {
            // Skip unknown chunk
            f.seek(f.pos() + sz);
        }
    }

    if (!hasFmt || dataPos < 0) {
        if (err) *err = "Missing fmt or data chunk";
        return false;
    }

    // Only PCM16 is supported
    if (audioFormat != 1 || bitsPerSample != 16) {
        if (err) *err = QString("Unsupported format: code=%1, bits=%2")
                            .arg(audioFormat).arg(bitsPerSample);
        return false;
    }

    // --- Read sample data ---
    f.seek(dataPos);
    QByteArray raw = f.read(dataSize);

    const int16_t* p = reinterpret_cast<const int16_t*>(raw.constData());
    int frames = dataSize / (numChannels * 2);

    out.clear();
    out.reserve(frames);

    if (numChannels == 1) {
        // Mono: copy directly
        out.assign(p, p + frames);
    } else {
        // Stereo: downmix to mono (average L & R)
        for (int i = 0; i < frames; i++) {
            int32_t L = p[2 * i];
            int32_t R = p[2 * i + 1];
            out.push_back(static_cast<int16_t>((L + R) / 2));
        }
    }

    // Fill WavInfo struct
    info.sampleRate = sampleRate;
    info.channels = numChannels;
    info.bitsPerSample = 16;
    info.totalFrames = frames;

    return true;
}

/// Save PCM16 mono samples into a WAV file
bool WavFile::saveMonoPcm16(const QString& path,
                            const std::vector<int16_t>& samples,
                            int sr,
                            QString* err) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly)) {
        if (err) *err = "Cannot write file";
        return false;
    }

    quint32 dataSize = samples.size() * 2;
    quint32 byteRate = sr * 2;

    // --- RIFF header ---
    char riff[4] = {'R','I','F','F'};
    quint32 riffSize = 36 + dataSize;
    char wave[4] = {'W','A','V','E'};

    // fmt chunk
    char fmtId[4] = {'f','m','t',' '};
    quint32 fmtSize = 16;
    quint16 audioFormat = 1; // PCM
    quint16 numChannels = 1;
    quint16 blockAlign = 2;
    quint16 bitsPerSample = 16;

    // data chunk
    char dataId[4] = {'d','a','t','a'};
    quint32 dataChunkSize = dataSize;

    // Write headers + audio data
    f.write(riff, 4);
    f.write(reinterpret_cast<const char*>(&riffSize), 4);
    f.write(wave, 4);

    f.write(fmtId, 4);
    f.write(reinterpret_cast<const char*>(&fmtSize), 4);
    f.write(reinterpret_cast<const char*>(&audioFormat), 2);
    f.write(reinterpret_cast<const char*>(&numChannels), 2);
    f.write(reinterpret_cast<const char*>(&sr), 4);
    f.write(reinterpret_cast<const char*>(&byteRate), 4);
    f.write(reinterpret_cast<const char*>(&blockAlign), 2);
    f.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    f.write(dataId, 4);
    f.write(reinterpret_cast<const char*>(&dataChunkSize), 4);
    f.write(reinterpret_cast<const char*>(samples.data()), dataSize);

    return true;
}