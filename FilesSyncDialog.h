#ifndef FILESSYNCDIALOG_H
#define FILESSYNCDIALOG_H

#include <QDialog>
#include "FileOperation.h"
#include <QColor>

namespace Ui {
    class FilesSyncDialog;
}

class FilesSyncDialog : public QDialog {
    Q_OBJECT

public:
    explicit FilesSyncDialog(QWidget *parent = 0);
    ~FilesSyncDialog();

    void push_back(const FileOperation& op);
private:
    Ui::FilesSyncDialog *ui;
    enum Columns {
        ColumnCheckBox = 0,
        ColumnSourceFileName = 1,
        ColumnInitialFileSize = 2,
        ColumnDifSize = 3,
        ColumnDifTime = 4
    };
    const QColor difSizeColor = QColor(255, 153,  51);
    const QColor difTimeColor = QColor(255, 255,   0);
    const QColor difAllColor  = QColor(255,  51,   0);

    void onSelAllClick();
    void onSelAllDifClick();
    void onSelDifSizeClick();
    void onSelDifTimeClick();
    void onSelNoneClick();

    void onEncryptSelClick();
signals:
    void setRestoreEncrypted(const std::string& encryptedName);
    void restoreEncrypted();
};

#endif // FILESSYNCDIALOG_H
