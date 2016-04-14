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
public slots:
    void onEncryptSelected();
    void onDecryptSelected();
    void onViewSelected();
    void onExecuteSelected();
    void onShowDecrypted();
    void onSetAsRoot();
    void onWipeSelected();
    void onOptionsClick();
    void onUpOneLevelClick();
    void onShowAbout();
private:
    void onRootPathChanged(const QString& newPath);
    void onCustomContextMenu(const QPoint& point);
    QMenu contextMenu;

    bool updateKeys();
    void updateKeyIv(std::string keyStr);

    enum class Operation {
        Encrypt,
        Decrypt,
        Wipe
    };

    void processOperation(Operation operation);
    int processItems(const std::vector<std::string>& names, std::vector<std::string>& unprocessedSrcNames, bool (MainWindow::*procFunc)(const std::string&));
    inline QFileSystemModel* model();

    CryptoPP::SecByteBlock key;
    CryptoPP::SecByteBlock iv;
    bool doEncrypt(const std::string& infile);
    bool doDecrypt(const std::string& infile);
    bool doRemove(const std::string& fileName);
    bool doCount(const std::string& fileName);

    Options options;
    OptionsDialog optionsDlg;
    WaitDialog waitDlg;
    FilesSyncDialog syncDlg;

    std::vector<FileOperation> fileOperations;
    // removes indicated file from fileOperations and from syncDlg.
    // bySourcePath == true -> name = sourceFileName
    // bySourcePath == false -> name = destinationFileName
    void removeFromFileOps(const std::string& name, bool bySourcePath);
    void closeEvent(QCloseEvent* event);

    void spawnFileViewer(const std::string& fileName);

public slots:
    void onMarkForProcess(const std::string& encryptedName);
    void onFilesOpEncryptedSelected(std::vector<std::string>& unprocessedSrcNames);
    void onDiscardAllFiles();
    void onFilesOpWipeSelected(std::vector<std::string>& unprocessedSrcNames);
};

#endif // MAINWINDOW_H
