#pragma once
#include <QDialog>

namespace Ui { class MetadataDialog; }

/**
 * @struct SongMeta
 * @brief Holds metadata about a song.
 *
 * Includes fields like title, artist, album, year, and genre.
 * The `isValid()` method ensures the required fields are filled
 * before storing in the database.
 */
struct SongMeta {
    QString title;
    QString artist;
    QString album;
    int year = 0;
    QString genre;

    /// Basic validation: require at least Title and Artist
    bool isValid() const {
        return !title.trimmed().isEmpty() && !artist.trimmed().isEmpty();
    }
};

/**
 * @class MetadataDialog
 * @brief Dialog window for entering song metadata.
 *
 * Pops up during the "Upload WAV" workflow so the user
 * can provide details (title, artist, album, year, genre)
 * before the track and fingerprints are saved to the database.
 */
class MetadataDialog : public QDialog {
    Q_OBJECT
public:
    explicit MetadataDialog(QWidget* parent=nullptr);
    ~MetadataDialog();

    /// Return collected metadata
    SongMeta meta() const;

private slots:
    void onOk();     ///< Validate inputs and accept dialog
    void onCancel(); ///< Cancel dialog without saving

private:
    Ui::MetadataDialog* ui; ///< Qt-generated UI form
    SongMeta m_meta;        ///< Stores user-provided metadata
};