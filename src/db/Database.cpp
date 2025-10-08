#include "Database.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

Database::Database(const QString& filePath) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(filePath);
}

/// Open database connection
bool Database::open(QString* err) {
    if (!m_db.open()) {
        if (err) *err = m_db.lastError().text();
        return false;
    }
    return true;
}

/// Create schema and indexes if not present
bool Database::migrate(QString* err) {
    QSqlQuery q(m_db);

    // Enable WAL mode for better concurrency
    q.exec("PRAGMA journal_mode=WAL;");

    // Songs table
    if (!q.exec("CREATE TABLE IF NOT EXISTS songs("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "title TEXT NOT NULL, artist TEXT NOT NULL,"
                "album TEXT, year INTEGER, genre TEXT)")) {
        if (err) *err = q.lastError().text();
        return false;
    }

    // Fingerprints table
    if (!q.exec("CREATE TABLE IF NOT EXISTS fingerprints("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "song_id INTEGER NOT NULL,"
                "hash INTEGER NOT NULL,"
                "offset_ms INTEGER NOT NULL)")) {
        if (err) *err = q.lastError().text();
        return false;
    }

    // Indexes for faster lookup
    if (!q.exec("CREATE INDEX IF NOT EXISTS idx_fp_hash ON fingerprints(hash)")) {
        if (err) *err = q.lastError().text();
        return false;
    }
    if (!q.exec("CREATE INDEX IF NOT EXISTS idx_fp_song ON fingerprints(song_id)")) {
        if (err) *err = q.lastError().text();
        return false;
    }

    return true;
}

/// Insert song metadata and return auto-generated ID
bool Database::insertSong(const SongRow& s, int& outId, QString* err) {
    QSqlQuery q(m_db);
    q.prepare("INSERT INTO songs(title,artist,album,year,genre) VALUES(?,?,?,?,?)");
    q.addBindValue(s.title);
    q.addBindValue(s.artist);
    q.addBindValue(s.album);
    q.addBindValue(s.year);
    q.addBindValue(s.genre);

    if (!q.exec()) {
        if (err) *err = q.lastError().text();
        return false;
    }

    outId = q.lastInsertId().toInt();
    return true;
}

/// Insert many fingerprints for a song (transaction for speed)
bool Database::insertFingerprints(int songId,
                                  const std::vector<std::pair<uint32_t,int>>& hashes,
                                  QString* err) {
    m_db.transaction();

    QSqlQuery q(m_db);
    q.prepare("INSERT INTO fingerprints(song_id,hash,offset_ms) VALUES(?,?,?)");

    for (auto& h : hashes) {
        q.addBindValue(songId);
        q.addBindValue((qulonglong)h.first);
        q.addBindValue(h.second);

        if (!q.exec()) {
            m_db.rollback();
            if (err) *err = q.lastError().text();
            return false;
        }

        // Re-prepare for next batch
        q.clear();
        q.prepare("INSERT INTO fingerprints(song_id,hash,offset_ms) VALUES(?,?,?)");
    }

    m_db.commit();
    return true;
}

/// Match fingerprints by voting mechanism
bool Database::bestMatch(const std::vector<std::pair<uint32_t,int>>& hashes,
                         SongRow& outSong,
                         int& voteCount,
                         QString* err) {
    // Votes keyed by (song_id, time delta)
    QHash<QPair<int,int>, int> votes;

    QSqlQuery q(m_db);
    q.prepare("SELECT song_id, offset_ms FROM fingerprints WHERE hash=?");

    // For each hash, look up candidates and vote
    for (auto& h : hashes) {
        q.addBindValue((qulonglong)h.first);

        if (!q.exec()) {
            if (err) *err = q.lastError().text();
            return false;
        }

        while (q.next()) {
            int song = q.value(0).toInt();
            int delta = q.value(1).toInt() - h.second;
            auto key = qMakePair(song, delta);
            votes[key] += 1;
        }

        q.finish();
        q.bindValue(0, QVariant()); // reset binding
    }

    // Pick song with highest vote count
    int bestSong = -1;
    int bestCount = 0;
    for (auto it = votes.constBegin(); it != votes.constEnd(); ++it) {
        if (it.value() > bestCount) {
            bestCount = it.value();
            bestSong = it.key().first;
        }
    }

    voteCount = bestCount;
    if (bestSong < 0) return false;

    // Fetch song metadata
    QSqlQuery q2(m_db);
    q2.prepare("SELECT id,title,artist,album,year,genre FROM songs WHERE id=?");
    q2.addBindValue(bestSong);

    if (!q2.exec() || !q2.next()) {
        if (err) *err = q2.lastError().text();
        return false;
    }

    outSong.id     = q2.value(0).toInt();
    outSong.title  = q2.value(1).toString();
    outSong.artist = q2.value(2).toString();
    outSong.album  = q2.value(3).toString();
    outSong.year   = q2.value(4).toInt();
    outSong.genre  = q2.value(5).toString();

    return true;
}