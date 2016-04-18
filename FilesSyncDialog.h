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
    explicit FilesSyncDialog(QWidget* parent = 0);
    ~FilesSyncDialog();

    void push_back(const FileOperation& op);
    void clear();
    void remove(const FileOperation& op);
private:
    Ui::FilesSyncDialog* ui;
    enum Columns {
        ColumnCheckBox = 0,
        ColumnSourceFileName = 1,
        /*
         * ColumnInitialFileSize = 2,
         * ColumnDifSize = 3,
         * ColumnDifTime = 4
         */
        ColumnDifSize = 2,
        ColumnDifTime = 3
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
    void onWipeSelClick();

    // Updates MainWindow's fileOperations - sets 'true' to process checked files
    // Returns number of updated fileOperations
    int updateFileOperations();
    void showNoFilesSelectedMsg() const;
    void closeEvent(QCloseEvent* event);
signals:
    // sets FileOperation field to process particular item
    void setMarkForProcess(const std::wstring& encryptedName);
    // restores selected encrypted files with restore flag
    void restoreEncryptedSelected(std::vector<std::wstring>& unprocessedSrcNames);
    // discards all file changes
    void discardAllFiles();
    // wipes selected files
    void wipeSelected(std::vector<std::wstring>& unprocessedSrcNames);
};

#endif // FILESSYNCDIALOG_H
