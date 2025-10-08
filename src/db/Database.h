#pragma once
#include <QString>
#include <QSqlDatabase>
#include <vector>
#include <cstdint>

/**
 * @struct SongRow
 * @brief Represents a song record in the database.
 */
struct SongRow {
    int id = -1;
    QString title, artist, album, genre;
    int year = 0;
};

/**
 * @class Database
 * @brief SQLite wrapper for storing songs and fingerprints.
 *
 * Schema:
 *   - songs(id, title, artist, album, year, genre)
 *   - fingerprints(id, song_id, hash, offset_ms)
 *
 * Features:
 *   - Migration (auto-create schema + indexes if missing)
 *   - Insert new songs with metadata
 *   - Insert fingerprint hashes (transaction for efficiency)
 *   - Find best match by hash voting (song_id + time delta)
 */
class Database {
public:
    explicit Database(const QString& filePath);

    /// Open SQLite database connection
    bool open(QString* err=nullptr);

    /// Create schema if missing, setup indexes
    bool migrate(QString* err=nullptr);

    /// Insert a new song row, returning its generated ID
    bool insertSong(const SongRow& s, int& outId, QString* err=nullptr);

    /// Insert fingerprint hashes for a given song (transactional)
    bool insertFingerprints(int songId,
                            const std::vector<std::pair<uint32_t,int>>& hashes,
                            QString* err=nullptr);

    /// Attempt to match a set of fingerprints against DB
    /// Uses (song_id, delta) voting to pick best candidate
    bool bestMatch(const std::vector<std::pair<uint32_t,int>>& hashes,
                   SongRow& outSong,
                   int& voteCount,
                   QString* err=nullptr);

private:
    QSqlDatabase m_db;
};