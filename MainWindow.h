#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <string>
#include "../cryptopp/secblock.h"

#include "OptionsDialog.h"
#include <QFileSystemModel>
#include "WaitDialog.h"

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

    void updateKeys();

    enum class Operation {
        Encrypt,
        Decrypt,
        Wipe
    };

    void processOperation(Operation operation);
    int processItem(const std::string& name, void (MainWindow::*procFunc)(const std::string&));
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
};

#endif // MAINWINDOW_H
