#include "MetadataDialog.h"
#include "ui_metadatadialog.h"
#include <QMessageBox>

MetadataDialog::MetadataDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::MetadataDialog) {
    ui->setupUi(this);

    // Connect buttons to handlers
    connect(ui->btnOK,     &QPushButton::clicked, this, &MetadataDialog::onOk);
    connect(ui->btnCancel, &QPushButton::clicked, this, &MetadataDialog::onCancel);
}

MetadataDialog::~MetadataDialog() { delete ui; }

/// OK button: collect inputs, validate, and close dialog
void MetadataDialog::onOk() {
    m_meta.title  = ui->editTitle->text();
    m_meta.artist = ui->editArtist->text();
    m_meta.album  = ui->editAlbum->text();
    m_meta.year   = ui->spinYear->value();
    m_meta.genre  = ui->editGenre->text();

    if (!m_meta.isValid()) {
        QMessageBox::warning(this, "Validation", "Title and Artist are required.");
        return; // Stay in dialog if invalid
    }

    accept(); // Close with Accepted result
}

/// Cancel button: discard inputs and close dialog
void MetadataDialog::onCancel() {
    reject();
}

/// Return metadata collected in this dialog
SongMeta MetadataDialog::meta() const {
    return m_meta;
}