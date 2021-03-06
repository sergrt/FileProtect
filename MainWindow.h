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
#include "InputKeyDialog.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow* ui;
public slots:
    void onEncryptSelected();
    void onDecryptSelected();
    void onViewSelected();
    void onExecuteSelected();
    int onShowDecrypted();
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
    void updateKeyIv(std::string& keyStr);

    enum class Operation {
        Encrypt,
        Decrypt,
        Wipe
    };

    void processOperation(Operation operation);
    int processItems(const std::vector<std::wstring>& names, std::vector<std::wstring>& unprocessedSrcNames, const std::wstring& relativePath, bool (MainWindow::*procFunc)(const std::wstring&, const std::wstring&));
    void showResultMsg(const std::vector<std::wstring>& unprocessedSrcNames) const;
    inline QFileSystemModel* model();

    CryptoPP::SecByteBlock key;
    CryptoPP::SecByteBlock iv;
    bool doEncrypt(const std::wstring& infile, const std::wstring& = L"");
    bool doDecrypt(const std::wstring& infile, const std::wstring& relativePath = L"");
    bool doRemove(const std::wstring& fileName, const std::wstring& = L"");
    bool doCount(const std::wstring& fileName, const std::wstring& = L"");

    Options options;
    OptionsDialog optionsDlg;
    WaitDialog waitDlg;
    FilesSyncDialog syncDlg;
    InputKeyDialog keyDlg;

    std::vector<FileOperation> fileOperations;
    // removes indicated file from fileOperations and from syncDlg.
    // bySourcePath == true -> name = sourceFileName
    // bySourcePath == false -> name = destinationFileName
    void removeFromFileOps(const std::wstring& name, bool bySourcePath);
    // Try to remove relative path for file fName
    void tryRemovePath(const std::wstring& fName, const std::wstring& relativePath);
    void closeEvent(QCloseEvent* event);

    void spawnFileViewer(const std::wstring& fileName);

public slots:
    void onMarkForProcess(const std::wstring& encryptedName);
    void onFilesOpEncryptedSelected(std::vector<std::wstring>& unprocessedSrcNames);
    void onDiscardAllFiles();
    void onFilesOpWipeSelected(std::vector<std::wstring>& unprocessedSrcNames);
};

#endif // MAINWINDOW_H
