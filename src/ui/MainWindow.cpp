#include "MainWindow.h"
#include "ui_mainwindow.h"
#include "audio/WavFile.h"
#include "fingerprint/Fingerprint.h"
#include "MetadataDialog.h"

#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_db("music.db") {
    ui->setupUi(this);

    // Connect UI buttons to their respective handlers
    connect(ui->btnUpload, &QPushButton::clicked, this, &MainWindow::onUpload);
    connect(ui->btnRecord, &QPushButton::clicked, this, &MainWindow::onRecord);
    connect(ui->btnPlay,   &QPushButton::clicked, this, &MainWindow::onPlay);
    connect(ui->btnStop,   &QPushButton::clicked, this, &MainWindow::onStop);

    // Signal: recording finished -> attempt recognition
    connect(&m_capture, &AudioCapture::finished, this, &MainWindow::onCaptureFinished);

    // Initialize and migrate database
    QString err;
    if (!m_db.open(&err) || !m_db.migrate(&err)) {
        QMessageBox::critical(this, "DB Error", err);
    }
}

MainWindow::~MainWindow() { delete ui; }

/// Append a result/status message to the results text box
void MainWindow::appendResult(const QString& s) {
    ui->txtResults->append(s);
}

/// Handle "Upload WAV..." button
void MainWindow::onUpload() {
    QString path = QFileDialog::getOpenFileName(this, "Select WAV", QString(), "WAV files (*.wav)");
    if (path.isEmpty()) return;
    fingerprintAndStore(path);
}

/// Load a WAV file, fingerprint it, and store song+hashes in DB
void MainWindow::fingerprintAndStore(const QString& wavPath) {
    std::vector<int16_t> samples;
    WavInfo info; QString err;

    // Load PCM16 samples from WAV file
    if (!WavFile::loadPcm16(wavPath, samples, info, &err)) {
        QMessageBox::warning(this, "WAV", "Failed to load WAV file: " + err);
        return;
    }

    // Compute fingerprints (hashes) from audio samples
    auto hashes = Fingerprint::compute(samples, info.sampleRate);
    appendResult(QString("Computed %1 hashes").arg(hashes.size()));

    // Prompt user for song metadata (title, artist, album, etc.)
    MetadataDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) {
        appendResult("Metadata cancelled; not storing.");
        return;
    }

    // Store song info in DB
    SongMeta m = dlg.meta();
    SongRow s;
    s.title = m.title; s.artist = m.artist; s.album = m.album;
    s.year = m.year;   s.genre = m.genre;

    int songId = -1;
    if (!m_db.insertSong(s, songId, &err)) {
        QMessageBox::warning(this, "DB", "Insert song failed: " + err); return;
    }

    // Store computed fingerprints in DB
    if (!m_db.insertFingerprints(songId, hashes, &err)) {
        QMessageBox::warning(this, "DB", "Insert fingerprints failed: " + err); return;
    }

    appendResult(QString("Stored song #%1: %2, by %3").arg(songId).arg(s.title, s.artist));

    // Keep audio buffer for playback
    m_loadedPcm = std::move(samples);
    m_loadedSr = info.sampleRate;
    m_player.setBuffer(m_loadedPcm, m_loadedSr);
}

/// Handle "Record" button: capture 10 seconds of audio
void MainWindow::onRecord() {
    appendResult("Recording for 10 seconds...");
    m_capture.start(10);
}

/// Callback when recording is finished
void MainWindow::onCaptureFinished() {
    appendResult("Recording finished. Recognizing...");
    recognizeFromBuffer(m_capture.samples(), m_capture.sampleRate());
}

/// Compute fingerprints from captured buffer and find best match in DB
void MainWindow::recognizeFromBuffer(const std::vector<int16_t>& pcm, int sr) {
    auto hashes = Fingerprint::compute(pcm, sr);

    QString err;
    SongRow best; int votes = 0;

    if (!m_db.bestMatch(hashes, best, votes, &err)) {
        appendResult("No match found.");
        return;
    }

    appendResult(QString("Match: %1, by %2  (votes = %3)")
                 .arg(best.title, best.artist).arg(votes));
}

/// Handle "Play" button
void MainWindow::onPlay() {
    if (!m_loadedPcm.empty()) m_player.play();
}

/// Handle "Stop" button
void MainWindow::onStop() {
    m_player.stop();
}