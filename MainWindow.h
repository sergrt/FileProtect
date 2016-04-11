#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include "../cryptopp/secblock.h"

#include "OptionsDialog.h"
#include <QFileSystemModel>
#include "WaitDialog.h"
#include <QMenu>
#include "FilesSyncDialog.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void onUpOneLevelClick();
    void onSetAsRoot();
    void onRootPathChanged(const QString& newPath);
    void onEncryptSelected();
    void onDecryptSelected();
    void onWipeSelected();
    void onOptionsClick();
    void onCustomContextMenu(const QPoint& point);
    QMenu contextMenu;

    void updateKeys();

    enum class Operation {
        Encrypt,
        Decrypt,
        Wipe
    };

    void processOperation(Operation operation);
    int processItems(const std::vector<std::string>& names, void (MainWindow::*procFunc)(const std::string&));
    inline QFileSystemModel* model();

    CryptoPP::SecByteBlock key;
    CryptoPP::SecByteBlock iv;
    void doEncrypt(const std::string& infile);
    void doDecrypt(const std::string& infile);
    void doRemove(const std::string& fileName);
    void doCount(const std::string& fileName);

    Options options;
    OptionsDialog optionsDlg;
    WaitDialog waitDlg;
    FilesSyncDialog syncDlg;

    std::vector<FileOperation> fileOperations;
    void closeEvent(QCloseEvent* event);
public slots:
    void onSetRestoreEncrypted(const std::string& encryptedName);
    void onRestoreEncrypted();
};

#endif // MAINWINDOW_H
