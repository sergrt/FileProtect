#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../cryptopp/modes.h"
#include "../cryptopp/files.h"
#include "../cryptopp/aes.h"


#include <QMessageBox>
const int dirFilter = QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;
#include <QProcess>
#include <QDateTime>
#include "CryptoppUtils.h"


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

    connect(ui->actionShow_decrypted, &QAction::triggered, this, [=]() {
        syncDlg.clear();
        for (const auto& op : fileOperations)
            syncDlg.push_back(op);
        syncDlg.show();
    });


    connect(&syncDlg, &FilesSyncDialog::setRestoreEncrypted, this, &MainWindow::onSetRestoreEncrypted);
    connect(&syncDlg, &FilesSyncDialog::restoreEncrypted, this, &MainWindow::onRestoreEncrypted);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateKeys() {
    std::string hexKey(CryptoPP::AES::MAX_KEYLENGTH,'0');
    std::string hexIv(CryptoPP::AES::BLOCKSIZE, '0');
    key = CryptoPPUtils::HexDecodeString(hexKey);
    iv = CryptoPPUtils::HexDecodeString(hexIv);
}

QFileSystemModel* MainWindow::model() {
    return static_cast<QFileSystemModel*>(ui->treeView->model());
}

void MainWindow::onCustomContextMenu(const QPoint& point) {
    QModelIndex index = ui->treeView->indexAt(point);
    if (index.isValid())
        contextMenu.exec(ui->treeView->mapToGlobal(point));
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

    std::vector<std::string> unprocessedSrcNames;
    const unsigned int totalCount = processItems(names, unprocessedSrcNames, &MainWindow::doCount);
    //waitDlg.hide();





    bool (MainWindow::*memberFn)(const std::string&) = nullptr;
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
    unprocessedSrcNames.clear();
    const unsigned int successfullyProcessed = processItems(names, unprocessedSrcNames, memberFn);

    if (operation == Operation::Wipe && successfullyProcessed == totalCount) { // do not remove dirs if not all files were wiped
        // This operation does not participate in resulting messageBox
        // beacuse elements of "names" could be subdirs of each other
        // and failing deleting them should not be considered as errors
        for (const auto& n : names) {
            QFileInfo fi(n.c_str());
            if (fi.exists() && fi.isDir()) {
                QDir d(n.c_str());
                d.removeRecursively();
            }
        }
    }
    waitDlg.hide();

    QMessageBox m;
    m.setText(QString("Operation finished. Total files processed: %1 of %2").arg(successfullyProcessed).arg(totalCount));
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
bool MainWindow::doEncrypt(const std::string& infile) {
    bool res = true;

    try {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption aes(key, key.size(), iv);
        std::string outfile = infile + ".aes";
        CryptoPP::FileSource(infile.c_str(), true, new CryptoPP::StreamTransformationFilter(aes, new CryptoPP::FileSink(outfile.c_str())));

        if (!doRemove(infile) || !QFile::rename(outfile.c_str(), infile.c_str()))
            throw std::runtime_error("Error file removing/renaming");
    } catch (std::exception&) {
        res = false;
    }

    return res;
}

bool MainWindow::doDecrypt(const std::string& infile) {
    bool res = true;


    try {
        std::string outfile = infile;
        if (options.decryptionPlace() == Options::DecryptionPlace::Specified) {
            QFileInfo fileInfo(infile.c_str());
            QString filename(fileInfo.fileName());

            QDir d(options.decryptionFolder().c_str());
            if (!d.exists() && !d.mkpath(options.decryptionFolder().c_str()))
                throw std::runtime_error("Error creating destination dir");

            outfile = d.absolutePath().toStdString() + "/" + filename.toStdString();
        } else {
            outfile += ".dec";
        }
        CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption aes(key, key.size(), iv);
        CryptoPP::FileSource(infile.c_str(), true, new CryptoPP::StreamTransformationFilter(aes, new CryptoPP::FileSink(outfile.c_str())));

        //
        QFile f(outfile.c_str());
        const unsigned long sz = f.size();
        QFileInfo fi(f);
        time_t timestamp = fi.lastModified().toTime_t();
        std::string fileOpOutName = outfile;
        //
        if (options.decryptionPlace() == Options::DecryptionPlace::Inplace) {
            if (!doRemove(infile) || !QFile::rename(outfile.c_str(), infile.c_str()))
                throw std::runtime_error("Error file removing/renaming");
            fileOpOutName = infile; // names are coincident
        }

        fileOperations.emplace_back(FileOperation(infile, fileOpOutName, sz, timestamp));
    } catch (std::exception&) {
        res = false;
    }

    return res;
}

bool MainWindow::doCount(const std::string& /*fileName*/) {
    // do nothing
    return true;
}

bool MainWindow::doRemove(const std::string& fileName) {
    bool res = true;
    if (options.wipeMethod() == Options::WipeMethod::Regular) {
        QFile::remove(fileName.c_str());
    } else /*if (options.wipeMethod() == Options::WipeMethod::External)*/ {
        const QString execStr = QString(options.wipeProgram().c_str()).arg(fileName.c_str());
        QProcess p;
        p.start(execStr);
        if (!p.waitForFinished())
            res = false;
    }

    return res;
}

int MainWindow::processItems(const std::vector<std::string>& names, std::vector<std::string>& unprocessedSrcNames, bool (MainWindow::*procFunc)(const std::string&)) {
    /*
    int res = 0;
    for (const auto& name : names) {
        const char* tmpName = name.c_str();
        QFileInfo f(tmpName);
        if (f.isDir()) {
            QDir dir(tmpName);
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
    */

    int res = 0;
    for (const auto& name : names) {
        const char* tmpName = name.c_str();
        QFileInfo f(tmpName);
        if (!f.isDir()) {
            if ((this->*procFunc)(f.absoluteFilePath().toStdString())) {
                ++res;
            } else {
                unprocessedSrcNames.push_back(name);
            }
        } else {
            QDir dir(tmpName);
            QFileInfoList fil = dir.entryInfoList(QDir::Filter(dirFilter));
            for (const auto& f : fil) {
                waitDlg.step();
                qApp->processEvents();

                std::vector<std::string> tmp;
                tmp.emplace_back(f.absoluteFilePath().toStdString());
                res += processItems(tmp, unprocessedSrcNames, procFunc);
            }
        }
    }
    return res;
}

void MainWindow::onOptionsClick() {
    optionsDlg.show();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (!fileOperations.empty()) {
        syncDlg.clear();
        for (const auto& op : fileOperations)
            syncDlg.push_back(op);

        syncDlg.show();
    }
}
void MainWindow::onSetRestoreEncrypted(const std::string& encryptedName) {
    for (auto& op : fileOperations) {
        if (encryptedName == op.sourcePathName)
            op.restoreEncrypted = true;
    }
}
#include <algorithm>
void MainWindow::onRestoreEncrypted() {
    std::vector<std::string> names;
    std::vector<std::string> namesToReplace;
    for (const auto& op : fileOperations) {
        if (op.restoreEncrypted) {
            names.emplace_back(op.destinationPathName);
            namesToReplace.emplace_back(op.sourcePathName);
        }
    }
    std::vector<std::string> unprocessedSrcNames;
    /*int processedCount = */processItems(names, unprocessedSrcNames, &MainWindow::doEncrypt);

    // removing encrypted files from filesOperations list
    for (const auto& n : namesToReplace) {
        if (std::find(unprocessedSrcNames.begin(), unprocessedSrcNames.end(), n) == unprocessedSrcNames.end()) {
            for (std::size_t i = 0; i < fileOperations.size(); ++i) {
                if (fileOperations[i].sourcePathName == n) {
                    FileOperation tmp(std::move(fileOperations[i]));
                    fileOperations.erase(fileOperations.begin() + i);
                    syncDlg.remove(tmp);
                    break;
                }
            }
        }
    }

    int filesLeft = names.size();
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (std::find(unprocessedSrcNames.begin(), unprocessedSrcNames.end(), namesToReplace[i]) == unprocessedSrcNames.end()) {
            // file was successfully encrypted, now it's time to restore its path if necessary
            if (names[i] == namesToReplace[i]) {
                --filesLeft;
            } else {
                QFile f(names[i].c_str());
                QFile d(namesToReplace[i].c_str());
                if (d.remove() && QFile::rename(names[i].c_str(), namesToReplace[i].c_str()))
                    --filesLeft;
            }
        }
    }

    if (filesLeft != 0) {
        QMessageBox m;
        QString msgText = QString("Error restoring files. %1 file(s) left in unencrypted state and %2 in unremoved state:")
                .arg(unprocessedSrcNames.size()).arg(filesLeft);
        m.setText(msgText);
        m.exec();
    }
}
