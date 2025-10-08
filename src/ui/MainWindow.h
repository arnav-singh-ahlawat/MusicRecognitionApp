#pragma once
#include <QMainWindow>
#include <vector>
#include <cstdint>
#include "audio/AudioPlayer.h"
#include "audio/AudioCapture.h"
#include "db/Database.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @class MainWindow
 * @brief The main GUI window for the music recognition app.
 *
 * Handles user interaction (upload, record, play, stop), and
 * ties together the UI, audio capture/playback, fingerprinting,
 * and database for storing/recognizing songs.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // UI event handlers
    void onUpload();           ///< Upload and fingerprint a WAV file
    void onRecord();           ///< Record audio from microphone
    void onPlay();             ///< Play last loaded/recorded audio
    void onStop();             ///< Stop playback
    void onCaptureFinished();  ///< Triggered after recording ends

private:
    void appendResult(const QString& s);                  ///< Append status text to results panel
    void fingerprintAndStore(const QString& wavPath);     ///< Fingerprint a WAV file and save to DB
    void recognizeFromBuffer(const std::vector<int16_t>& pcm, int sr); ///< Match audio against DB

    Ui::MainWindow *ui;   ///< Qt UI components
    AudioPlayer m_player; ///< Handles audio playback
    AudioCapture m_capture; ///< Manages microphone recording
    Database m_db;        ///< SQLite wrapper for songs & fingerprints

    // Buffers for the most recently loaded/recorded audio
    std::vector<int16_t> m_loadedPcm;
    int m_loadedSr = 44100;
};