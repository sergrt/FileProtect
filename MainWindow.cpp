#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../cryptopp/modes.h"
#include "../cryptopp/files.h"
#include "../cryptopp/aes.h"
#include "../cryptopp/hex.h"

#include <QMessageBox>
const int dirFilter = QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;
#include <QProcess>

CryptoPP::SecByteBlock HexDecodeString(const std::string& hexStr) {
    CryptoPP::StringSource ss(hexStr, true, new CryptoPP::HexDecoder);
    CryptoPP::SecByteBlock result((size_t)ss.MaxRetrievable());
    ss.Get(result, result.size());
    return result;
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), optionsDlg(options) {
    ui->setupUi(this);

    QFileSystemModel* model = new QFileSystemModel;
    connect(model, &QFileSystemModel::rootPathChanged, this, &MainWindow::onRootPathChanged);

    model->setRootPath(QDir::currentPath());
    //model->setFilter(QDir::AllEntries);
    connect(ui->actionUp_one_level, &QAction::triggered, this, &MainWindow::onUpOneLevelClick);
    connect(ui->actionSet_as_root, &QAction::triggered, this, &MainWindow::onSetAsRoot);
    connect(ui->actionEncrypt_selected, &QAction::triggered, this, &MainWindow::onEncryptSelected);
    connect(ui->actionDecrypt_selected, &QAction::triggered, this, &MainWindow::onDecryptSelected);
    connect(ui->actionWipe_selected, &QAction::triggered, this, &MainWindow::onWipeSelected);

    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::onOptionsClick);

    optionsDlg.hide();

    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(QDir::currentPath()));
    ui->treeView->setColumnWidth(0, 200);


    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::onCustomContextMenu);

    contextMenu.addAction(ui->actionEncrypt_selected);
    contextMenu.addAction(ui->actionDecrypt_selected);
    contextMenu.addAction(ui->actionWipe_selected);

    waitDlg.hide();


}

void MainWindow::updateKeys() {
    std::string hexKey(CryptoPP::AES::MAX_KEYLENGTH,'0');
    std::string hexIv(CryptoPP::AES::BLOCKSIZE, '0');
    key = HexDecodeString(hexKey);
    iv = HexDecodeString(hexIv);
}
QFileSystemModel* MainWindow::model() {
    return static_cast<QFileSystemModel*>(ui->treeView->model());
}

void MainWindow::onCustomContextMenu(const QPoint& point) {
    QModelIndex index = ui->treeView->indexAt(point);
    if (index.isValid()) {
        contextMenu.exec(ui->treeView->mapToGlobal(point));
    }
}

void MainWindow::onUpOneLevelClick() {
    QModelIndex currentRoot = ui->treeView->rootIndex();
    ui->treeView->setRootIndex(currentRoot.parent());
    onRootPathChanged(model()->filePath(currentRoot));
}
void MainWindow::onSetAsRoot() {
    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (model()->isDir(currentIndex))
        ui->treeView->setRootIndex(currentIndex);
    else
        ui->treeView->setRootIndex(currentIndex.parent());

    onRootPathChanged(model()->filePath(currentIndex));
}

void MainWindow::onRootPathChanged(const QString &newPath) {
    ui->currentRoot->setText(newPath);
}
void MainWindow::processOperation(Operation operation) {

    waitDlg.init(0);
    waitDlg.show();
    std::vector<std::string> names;

    QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
    foreach(QModelIndex index, list)
        names.emplace_back(model()->filePath(index).toStdString());

    const unsigned int totalCount = processItems(names, &MainWindow::doCount);
    //waitDlg.hide();





    void (MainWindow::*memberFn)(const std::string&) = nullptr;
    switch (operation) {
    case Operation::Encrypt:
        memberFn = &MainWindow::doEncrypt;
        updateKeys();
        break;
    case Operation::Decrypt:
        memberFn = &MainWindow::doDecrypt;
        updateKeys();
        break;
    case Operation::Wipe:
        memberFn = &MainWindow::doRemove;
        break;
    }


    waitDlg.init(totalCount);
    //waitDlg.show();
    const unsigned int filesCount = processItems(names, memberFn);

    /*
    if (operation == Operation::Wipe) {
        // rework
        QProcess p;
        std::string fileName = model()->fileName(currentIndex).toStdString();
        std::string filePath = model()->filePath(currentIndex).toStdString();
        const std::string path = fileName.substr(0, filePath.size() - fileName.size());
        const std::string cmd = "ls -rf \"" + path + "\"";
        p.startDetached(cmd.c_str());
        p.waitForFinished();
    }
    */
    waitDlg.hide();

    QMessageBox m;
    m.setText(QString("Operation successfully finished. Total files processed: %1 of %2").arg(filesCount).arg(totalCount));
    m.setIcon(QMessageBox::Icon::Information);
    m.setWindowTitle("Operation finished");
    m.setStandardButtons(QMessageBox::StandardButton::Ok);
    m.exec();
}

void MainWindow::onEncryptSelected() {
    processOperation(Operation::Encrypt);
}
void MainWindow::onDecryptSelected() {
    processOperation(Operation::Decrypt);
}
void MainWindow::onWipeSelected() {
    processOperation(Operation::Wipe);
}
void MainWindow::doEncrypt(const std::string& infile) {
    CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption aes(key, key.size(), iv);
    std::string outfile = infile + ".aes";
    CryptoPP::FileSource(infile.c_str(), true, new CryptoPP::StreamTransformationFilter(aes, new CryptoPP::FileSink(outfile.c_str())));

    doRemove(infile);
    QFile f(outfile.c_str());
    f.rename(infile.c_str());
}

void MainWindow::doDecrypt(const std::string& infile) {
    CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption aes(key, key.size(), iv);

    std::string outfile;
    if (options.decryptionPlace() == Options::DecryptionPlace::Specified) {
        QFileInfo fileInfo(infile.c_str());
        QString filename(fileInfo.fileName());
        outfile = options.decryptionFolder() + filename.toStdString() + ".11";
    }
    infile + ".dec";
    CryptoPP::FileSource(infile.c_str(), true, new CryptoPP::StreamTransformationFilter(aes, new CryptoPP::FileSink(outfile.c_str())));

    doRemove(infile);
    QFile f(outfile.c_str());
    f.rename(infile.c_str());
}

void MainWindow::doCount(const std::string& /*fileName*/) {
    // do nothing
}

void MainWindow::doRemove(const std::string& fileName) {
    if (options.wipeMethod() == Options::WipeMethod::Regular) {
        QFile f(fileName.c_str());
        f.remove();
    } else if (options.wipeMethod() == Options::WipeMethod::External) {
        const QString execStr = QString(options.wipeProgram().c_str()).arg(fileName.c_str());
        QProcess p;
        p.startDetached(execStr);
        p.waitForFinished();
    }
}

int MainWindow::processItems(const std::vector<std::string>& names, void (MainWindow::*procFunc)(const std::string&)) {
    int res = 0;
    for (const auto& name : names) {
        const char* tmpName = name.c_str();
        QFileInfo f(tmpName);
        if (f.isDir()) {
            QDir dir(tmpName);
            //QFileInfoList fil = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
            QFileInfoList fil = dir.entryInfoList(QDir::Filter(dirFilter));
            for (const auto& f : fil) {
                waitDlg.step();
                qApp->processEvents();

                if (f.isDir()) {
                    std::vector<std::string> tmp;
                    tmp.emplace_back(f.absoluteFilePath().toStdString());
                    res += processItems(tmp, procFunc);
                } else {
                    (this->*procFunc)(f.absoluteFilePath().toStdString());
                    ++res;
                }
            }
        } else {
            (this->*procFunc)(f.absoluteFilePath().toStdString());
            ++res;
        }
    }
    return res;
}
/*
void MainWindow::encrypt(const std::string &name) {
    const char* tmpName = name.c_str();
    QFileInfo f(tmpName);
    if (f.isDir()) {
        QDir dir(tmpName);
        QFileInfoList fil = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
        for (const auto& f : fil) {
            if (f.isDir()) {
                encrypt(f.absoluteFilePath().toStdString());
            } else {
                std::string infile = f.absoluteFilePath().toStdString();
                doEncrypt(infile);
            }
        }
    }
}

void MainWindow::decrypt(const std::string &name) {
    const char* tmpName = name.c_str();
    QFileInfo f(tmpName);
    if (f.isDir()) {
        QDir dir(tmpName);
        QFileInfoList fil = dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
        for (const auto& f : fil) {
            if (f.isDir()) {
                encrypt(f.absoluteFilePath().toStdString());
            } else {
                std::string infile = f.absoluteFilePath().toStdString();
                doDecrypt(infile);
            }
        }
    }
}
*/
MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::onOptionsClick() {
    optionsDlg.show();
}
