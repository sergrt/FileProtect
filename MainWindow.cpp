#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "../cryptopp/modes.h"
#include "../cryptopp/files.h"
#include "../cryptopp/aes.h"

#include <QMessageBox>
#include <QProcess>
#include <QDateTime>
#include "CryptoppUtils.h"
#include <algorithm>
#include <QCloseEvent>
#include <QShortcut>
#include <QDesktopServices>
#include "ViewFileDialog.h"
#include <QDesktopWidget>
#include "../cryptopp/sha.h"
#include <random>
#include "AboutDialog.h"

const int dirFilter = QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), optionsDlg(options) {
    ui->setupUi(this);

    QFileSystemModel* model = new QFileSystemModel;
    connect(model, &QFileSystemModel::rootPathChanged, this, &MainWindow::onRootPathChanged);

    //model->setFilter(QDir::AllEntries);
    connect(ui->actionUp_one_level, &QAction::triggered, this, &MainWindow::onUpOneLevelClick);
    connect(ui->actionSet_as_root, &QAction::triggered, this, &MainWindow::onSetAsRoot);
    connect(ui->actionEncrypt_selected, &QAction::triggered, this, &MainWindow::onEncryptSelected);
    connect(ui->actionDecrypt_selected, &QAction::triggered, this, &MainWindow::onDecryptSelected);
    connect(ui->actionWipe_selected, &QAction::triggered, this, &MainWindow::onWipeSelected);
    connect(ui->actionOptions, &QAction::triggered, this, &MainWindow::onOptionsClick);

    optionsDlg.hide();

    const std::wstring rootPath = options.rootPath();
    model->setRootPath(rootPath.size() > 0 ? QString::fromStdWString(rootPath) : QDir::currentPath());

    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->index(model->rootPath()));
    ui->treeView->setColumnWidth(0, 200);


    ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::onCustomContextMenu);

    contextMenu.addAction(ui->actionEncrypt_selected);
    contextMenu.addAction(ui->actionDecrypt_selected);
    contextMenu.addAction(ui->actionWipe_selected);

    waitDlg.hide();

    connect(ui->actionShow_decrypted, &QAction::triggered, this, &MainWindow::onShowDecrypted);


    connect(&syncDlg, &FilesSyncDialog::setMarkForProcess, this, &MainWindow::onMarkForProcess);
    connect(&syncDlg, &FilesSyncDialog::restoreEncryptedSelected, this, &MainWindow::onFilesOpEncryptedSelected);
    connect(&syncDlg, &FilesSyncDialog::wipeSelected, this, &MainWindow::onFilesOpWipeSelected);
    connect(&syncDlg, &FilesSyncDialog::discardAllFiles, this, &MainWindow::onDiscardAllFiles);

    // These shortcuts will be deleted automatically on app exit
    new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(onShowAbout()));
    new QShortcut(QKeySequence(Qt::Key_F2), this, SLOT(onSetAsRoot()));
    new QShortcut(QKeySequence(Qt::Key_F3), this, SLOT(onViewSelected()));
    new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(onExecuteSelected()));
    new QShortcut(QKeySequence(Qt::Key_F5), this, SLOT(onEncryptSelected()));
    new QShortcut(QKeySequence(Qt::Key_F6), this, SLOT(onDecryptSelected()));
    new QShortcut(QKeySequence(Qt::Key_F7), this, SLOT(onShowDecrypted()));
    new QShortcut(QKeySequence(Qt::Key_F8), this, SLOT(onWipeSelected()));
    new QShortcut(QKeySequence(Qt::Key_F9), this, SLOT(onOptionsClick()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp), this, SLOT(onUpOneLevelClick()));

    connect(ui->bnF2SetAsRoot,     &QPushButton::clicked, this, &MainWindow::onSetAsRoot);
    connect(ui->bnF3View,          &QPushButton::clicked, this, &MainWindow::onViewSelected);
    connect(ui->bnF4Execute,       &QPushButton::clicked, this, &MainWindow::onExecuteSelected);
    connect(ui->bnF5Encrypt,       &QPushButton::clicked, this, &MainWindow::onEncryptSelected);
    connect(ui->bnF6Decrypt,       &QPushButton::clicked, this, &MainWindow::onDecryptSelected);
    connect(ui->bnF7ShowDecrypted, &QPushButton::clicked, this, &MainWindow::onShowDecrypted);
    connect(ui->bnF8Wipe,          &QPushButton::clicked, this, &MainWindow::onWipeSelected);
    connect(ui->bnF9Options,       &QPushButton::clicked, this, &MainWindow::onOptionsClick);
    connect(ui->bnCtrlPgUpGoUp,    &QPushButton::clicked, this, &MainWindow::onUpOneLevelClick);

    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::onShowAbout);

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::updateKeyIv(std::string keyStr) {
    byte digest[CryptoPP::SHA256::DIGESTSIZE];
    CryptoPP::SHA256().CalculateDigest(digest, (byte*)keyStr.c_str(), keyStr.size());

    key.resize(CryptoPP::AES::MAX_KEYLENGTH);
    if (CryptoPP::SHA256::DIGESTSIZE != key.size())
        throw "CryptoPP integrity questioned - AES MAX_KEYLENGTH != SHA256 DIGESTSIZE";
    memcpy(key.BytePtr(), digest, CryptoPP::AES::MAX_KEYLENGTH);

    std::reverse(keyStr.begin(), keyStr.end());
    CryptoPP::SHA256().CalculateDigest(digest, (byte*)keyStr.c_str(), keyStr.size());
    iv.resize(CryptoPP::AES::BLOCKSIZE);
    memcpy(iv.BytePtr(), digest, CryptoPP::AES::BLOCKSIZE);
}

bool MainWindow::updateKeys() {
    bool res = false;
    if (options.keyStorage() == Options::KeyStorage::Keyboard) {
        std::string keyStr;

        if (keyDlg.keyStored()) {
            keyStr = keyDlg.getKey();
        } else {
            if (keyDlg.exec() == QDialog::Accepted)
                keyStr = keyDlg.getKey();
            keyDlg.clearUi();
        }

        if (keyStr.size() > 0) {
            updateKeyIv(keyStr);
            CryptoPP::SecureWipeArray(const_cast<char*>(keyStr.c_str()), keyStr.size());
            res = true;
        }
    } else /*if (options.keyStorage() == Options::KeyStorage::File)*/ {
        QFile f(QString::fromStdWString(options.keyFile()));
        if (f.open(QFile::ReadOnly)) {
            QByteArray b = f.readAll();
            f.close();
            std::string keyStr(b.data(), b.size());
            if (keyStr.size() > 0) {
                updateKeyIv(keyStr);
                res = true;
            }
        }
    }
    return res;
}

QFileSystemModel* MainWindow::model() {
    return static_cast<QFileSystemModel*>(ui->treeView->model());
}

void MainWindow::onCustomContextMenu(const QPoint& point) {
    QModelIndex index = ui->treeView->indexAt(point);
    if (index.isValid())
        contextMenu.exec(ui->treeView->mapToGlobal(point));
}

void MainWindow::spawnFileViewer(const std::wstring& fileName) {
    ViewFileDialog* d = new ViewFileDialog(/*this*/);
    d->setAttribute(Qt::WA_DeleteOnClose);
    const QString fileNameFw = QString::fromStdWString(fileName);
    d->setWindowTitle(fileNameFw);
    {
        const QRect desktopRect = QApplication::desktop()->screenGeometry();
        const int w = desktopRect.width() / 3;
        const int h = desktopRect.height() / 1.5;

        // Adding shifting to window pos to avoid complete
        // overlapping if previous was not moved anywhere
        const int shiftPx = 24;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(-5, 5);
        const int wcx = dis(gen) * shiftPx;
        const int wcy = dis(gen) * shiftPx;

        d->setGeometry((desktopRect.right() - desktopRect.left()) / 2 - w / 2 + wcx,
                      (desktopRect.bottom() - desktopRect.top()) / 2 - h / 2 + wcy,
                       w, h);
    }

    QFile f(fileNameFw);
    if (f.open(QFile::ReadOnly)) {
        QByteArray b = f.readAll();
        f.close();
        d->setText(b);
    }

    d->show();
}

void MainWindow::onViewSelected() {
    QModelIndex m = ui->treeView->currentIndex();
    if (m.isValid())
        spawnFileViewer(model()->filePath(m).toStdWString());
}

void MainWindow::onExecuteSelected() {
    QDesktopServices::openUrl(QUrl(model()->filePath(ui->treeView->currentIndex())));
}

void MainWindow::onShowDecrypted() {
    syncDlg.clear();
    for (const auto& op : fileOperations)
        syncDlg.push_back(op);

    const QRect desktopRect = QApplication::desktop()->screenGeometry();
    syncDlg.setGeometry((desktopRect.right() - desktopRect.left()) / 2 - syncDlg.width() / 2,
                  (desktopRect.bottom() - desktopRect.top()) / 2 - syncDlg.height() / 2,
                   syncDlg.width(), syncDlg.height());

    if (syncDlg.exec() == QDialog::Rejected)
        fileOperations.clear();
}

void MainWindow::onUpOneLevelClick() {
    QModelIndex currentRoot = ui->treeView->rootIndex();
    if (currentRoot.isValid()) {
        QModelIndex parent = currentRoot.parent();
        const QString rootPath = model()->filePath(parent);
        model()->setRootPath(rootPath);
        ui->treeView->setRootIndex(parent);
    }
}

void MainWindow::onSetAsRoot() {
    /*
    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (model()->isDir(currentIndex))
        ui->treeView->setRootIndex(currentIndex);
    else
        ui->treeView->setRootIndex(currentIndex.parent());

    onRootPathChanged(model()->filePath(currentIndex));
    */

    QModelIndex currentIndex = ui->treeView->currentIndex();
    if (currentIndex.isValid()) {
        if (!model()->isDir(currentIndex))
            currentIndex = currentIndex.parent();
        if (currentIndex.isValid()) {
            const QString rootPath = model()->filePath(currentIndex);
            model()->setRootPath(rootPath);
            ui->treeView->setRootIndex(currentIndex);
        }
    }
}

void MainWindow::onRootPathChanged(const QString &newPath) {
    ui->currentRoot->setText(newPath);
    options.setRootPath(newPath.toStdWString());
    options.save();
}

void MainWindow::processOperation(Operation operation) {
    try {
        if (operation != Operation::Wipe && !updateKeys())
            throw std::logic_error("Unable to obtain key");

        waitDlg.init(0);
        waitDlg.show();
        std::vector<std::wstring> names;

        QModelIndexList list = ui->treeView->selectionModel()->selectedRows();
        foreach(QModelIndex index, list)
            names.emplace_back(model()->filePath(index).toStdWString());

        std::vector<std::wstring> unprocessedSrcNames;
        const unsigned int totalCount = processItems(names, unprocessedSrcNames, L"/", &MainWindow::doCount);
        //waitDlg.hide();

        bool (MainWindow::*memberFn)(const std::wstring&, const std::wstring&) = nullptr;
        switch (operation) {
        case Operation::Encrypt:
            memberFn = &MainWindow::doEncrypt;
            break;
        case Operation::Decrypt:
            memberFn = &MainWindow::doDecrypt;
            break;
        case Operation::Wipe:
            memberFn = &MainWindow::doRemove;
            break;
        }

        waitDlg.init(totalCount);
        //waitDlg.show();
        unprocessedSrcNames.clear();
        const unsigned int successfullyProcessed = processItems(names, unprocessedSrcNames, L"/", memberFn);

        if (operation == Operation::Wipe && successfullyProcessed == totalCount) { // do not remove dirs if not all files were wiped
            // This operation does not participate in resulting messageBox
            // beacuse elements of "names" could be subdirs of each other
            // and failing deleting them should not be considered as errors
            for (const auto& n : names) {
                const QString nameFw = QString::fromStdWString(n);
                QFileInfo fi(nameFw);
                if (fi.exists() && fi.isDir()) {
                    QDir d(nameFw);
                    d.removeRecursively();
                }
            }
        }
        waitDlg.hide();

        showResultMsg(unprocessedSrcNames);
    } catch (std::exception& e) {
        if (waitDlg.isVisible())
            waitDlg.hide();
        QMessageBox m;
        m.setText("Error: " + QString(e.what()));
        m.setIcon(QMessageBox::Critical);
        m.setWindowTitle("Operation finished");
        m.setStandardButtons(QMessageBox::StandardButton::Ok);
        m.exec();
    }
}

void MainWindow::showResultMsg(const std::vector<std::wstring>& unprocessedSrcNames) const {
    QMessageBox m;
    m.setWindowTitle("Operation summary");
    QString msgText;

    if (unprocessedSrcNames.size() > 0) {
        msgText = QString("Error processing files. %1 file(s) left unprocessed").arg(unprocessedSrcNames.size());
        const size_t maxFilesToOutput = 10;
        const size_t maxCount = std::min(maxFilesToOutput, unprocessedSrcNames.size());
        for (std::size_t i = 0; i < maxCount; ++i)
            msgText += "\n\t" + QString::fromStdWString(unprocessedSrcNames[i]);

        if (maxCount < unprocessedSrcNames.size())
            msgText += "\n\t...";

        m.setIcon(QMessageBox::Warning);
    } else {
        msgText = "Operation completed";
        m.setIcon(QMessageBox::Information);
    }
    m.setText(msgText);
    m.exec();
}

void MainWindow::onEncryptSelected() {
    processOperation(Operation::Encrypt);
}
void MainWindow::onDecryptSelected() {
    processOperation(Operation::Decrypt);
}
void MainWindow::onWipeSelected() {
    QMessageBox m;
    m.setText("You are about to wipe file(s). Note that this is potentially irreversible operation. Are you sure you want to proceed?");
    m.setWindowTitle("Confirmation");
    m.setIcon(QMessageBox::Warning);
    m.setStandardButtons(QMessageBox::No | QMessageBox::Yes);
    m.setDefaultButton(QMessageBox::No);
    if (m.exec() == QMessageBox::Yes)
        processOperation(Operation::Wipe);

}
bool MainWindow::doEncrypt(const std::wstring& infile, const std::wstring&) {
    bool res = true;

    try {
        CryptoPP::CTR_Mode<CryptoPP::AES>::Encryption aes(key, key.size(), iv);
        std::wstring outfile = infile + L"~aes";
        CryptoPP::FileSource(infile.c_str(), true, new CryptoPP::StreamTransformationFilter(aes, new CryptoPP::FileSink(outfile.c_str())));

        if (!doRemove(infile) || !QFile::rename(QString::fromStdWString(outfile), QString::fromStdWString(infile)))
            throw std::runtime_error("Error file removing/renaming");

        removeFromFileOps(infile, true);
    } catch (std::exception&) {
        res = false;
    }

    return res;
}

bool MainWindow::doDecrypt(const std::wstring& infile, const std::wstring& relativePath) {
    bool res = true;

    try {
        std::wstring outfile = infile;
        if (options.decryptionPlace() == Options::DecryptionPlace::Specified) {
            QFileInfo fileInfo(QString::fromStdWString(infile));
            QString filename(fileInfo.fileName());

            const QString decryptionFolderFw = QString::fromStdWString(options.decryptionFolder());
            QDir d(decryptionFolderFw);
            if (!d.exists() && !d.mkpath(decryptionFolderFw))
                throw std::runtime_error("Error creating destination dir");

            // Create relative dir
            const std::wstring fullFileDir = d.absolutePath().toStdWString() + (relativePath.size() > 0 ? relativePath : L"");
            QDir tmpDir(QString::fromStdWString(fullFileDir));
            if (!tmpDir.exists() && !tmpDir.mkpath(QString::fromStdWString(fullFileDir)))
                throw std::runtime_error("Unable to create relative dir");
            // It is safe to use "/" here - Qt will translate it to correct path symbol
            outfile = fullFileDir + L"/" + filename.toStdWString();
        } else {
            outfile += L"~dec";
        }
        CryptoPP::CTR_Mode<CryptoPP::AES>::Decryption aes(key, key.size(), iv);
        CryptoPP::FileSource(infile.c_str(), true, new CryptoPP::StreamTransformationFilter(aes, new CryptoPP::FileSink(outfile.c_str())));

        QFileInfo fi(QString::fromStdWString(outfile));
        const unsigned long sz = fi.size();
        time_t timestamp = fi.lastModified().toTime_t();
        std::wstring fileOpOutName = outfile;
        if (options.decryptionPlace() == Options::DecryptionPlace::Inplace) {
            if (!doRemove(infile) || !QFile::rename(QString::fromStdWString(outfile), QString::fromStdWString(infile)))
                throw std::runtime_error("Error file removing/renaming");
            fileOpOutName = infile; // names are coincident
        }

        if (std::find_if(fileOperations.begin(), fileOperations.end(), [&infile](const FileOperation& o)->bool {
               return o.sourcePathName == infile;
            }) == fileOperations.end()) {
            fileOperations.emplace_back(FileOperation(infile, fileOpOutName, sz, timestamp, relativePath));
            //syncDlg.push_back(fileOperations[fileOperations.size() - 1]);
        }
    } catch (std::exception&) {
        res = false;
    }

    return res;
}

bool MainWindow::doCount(const std::wstring&/* fileName*/, const std::wstring&) {
    return true;
}

bool MainWindow::doRemove(const std::wstring& fileName, const std::wstring&) {
    bool res = true;
    if (options.wipeMethod() == Options::WipeMethod::Regular) {
        QFile::remove(QString::fromStdWString(fileName));
    } else /*if (options.wipeMethod() == Options::WipeMethod::External)*/ {
        const QString execStr = QString(QString::fromStdWString(options.wipeProgram())).arg(QString::fromStdWString(fileName));
        QProcess p;
        p.start(execStr);
        if (!p.waitForFinished())
            res = false;
    }
    if (res)
        removeFromFileOps(fileName, true);

    return res;
}

int MainWindow::processItems(const std::vector<std::wstring>& names, std::vector<std::wstring>& unprocessedSrcNames, const std::wstring& relativePath, bool (MainWindow::*procFunc)(const std::wstring&, const std::wstring&)) {
    int res = 0;
    for (const auto& name : names) {
        const QString tmpName = QString::fromStdWString(name);
        QFileInfo f(tmpName);
        if (!f.isDir()) {
            waitDlg.step();
            qApp->processEvents();

            if ((this->*procFunc)(f.absoluteFilePath().toStdWString(), relativePath))
                ++res;
            else
                unprocessedSrcNames.push_back(name);
        } else {
            QDir dir(tmpName);
            std::wstring lastDirName = dir.path().toStdWString();
            const std::size_t delimPos = lastDirName.rfind(L"/");
            if (delimPos == std::wstring::npos) {
                throw std::runtime_error("Relative path is invalid");
            } else {
                lastDirName = lastDirName.substr(delimPos + 1, lastDirName.size() - delimPos - 1);

                const std::wstring newRelativePath = relativePath
                    + ((relativePath.size() > 0 && relativePath[relativePath.size() - 1] != L'/') ? L"/" : L"")
                    + lastDirName;
                QFileInfoList fil = dir.entryInfoList(QDir::Filter(dirFilter));
                for (const auto& f : fil) {
                    std::vector<std::wstring> tmp;
                    tmp.emplace_back(f.absoluteFilePath().toStdWString());
                    res += processItems(tmp, unprocessedSrcNames, newRelativePath, procFunc);
                }
            } // relative path ok
        } // f is dir
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

        if (syncDlg.exec() != QDialog::Rejected)
            event->ignore();
    }
}
void MainWindow::onMarkForProcess(const std::wstring& encryptedName) {
    for (auto& op : fileOperations) {
        if (encryptedName == op.sourcePathName)
            op.processItem = true;
    }
}

QString getTopLevelRelPath(const std::wstring& fName, const std::wstring& relativePath) {
    // Check if relative path is empty and delete path
    QFileInfo f(QString::fromStdWString(fName));
    QString topLevelRelPath = f.absolutePath();
    topLevelRelPath = topLevelRelPath.left(topLevelRelPath.size() - relativePath.size());

    const std::size_t delimPos = relativePath.find(L'/', 1);
    if (delimPos != std::wstring::npos)
        topLevelRelPath += QString::fromStdWString(relativePath.substr(0, delimPos));
    else if (relativePath.size() != 1)
        topLevelRelPath += QString::fromStdWString(relativePath);

    return topLevelRelPath;
}

void MainWindow::onFilesOpEncryptedSelected(std::vector<std::wstring>& unprocessedSrcNames) {
    waitDlg.init(0);
    waitDlg.show();

    std::vector<std::wstring> names;
    std::vector<std::wstring> namesToReplace;
    std::vector<std::wstring> relativePaths;
    for (const auto& op : fileOperations) {
        if (op.processItem) {
            names.emplace_back(op.destinationPathName);
            namesToReplace.emplace_back(op.sourcePathName);
            relativePaths.emplace_back(op.relativePath);
        }
    }
    //std::vector<std::string> unprocessedSrcNames;
    waitDlg.init(names.size());
    unprocessedSrcNames.clear();
    /*int processedCount = */processItems(names, unprocessedSrcNames, L"/", &MainWindow::doEncrypt);



    /*
    std::size_t filesLeft = names.size();
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
    */
    // delete and rename processed files. If this operation fails, put file in unprocessed vector
    std::size_t filesLeft = names.size();
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (std::find(unprocessedSrcNames.begin(), unprocessedSrcNames.end(), namesToReplace[i]) == unprocessedSrcNames.end()) {
            // file was successfully encrypted, now it's time to restore its path if necessary
            if (names[i] == namesToReplace[i]) {
                --filesLeft;
            } else {
                const QString curName = QString::fromStdWString(names[i]);
                const QString curNameToReplace = QString::fromStdWString(namesToReplace[i]);
                QFile d(curNameToReplace);
                if (d.remove() && QFile::rename(curName, curNameToReplace)) {
                    --filesLeft;

                    const QString topLevelRelPath = getTopLevelRelPath(curName.toStdWString(), relativePaths[i]);
                    if (topLevelRelPath.size() != 0) {
                        std::vector<std::wstring> names, unprocessedSrcNames;
                        names.push_back(topLevelRelPath.toStdWString());
                        if (processItems(names, unprocessedSrcNames, L"/", &MainWindow::doCount) == 0) {
                            QDir d(topLevelRelPath);
                            d.removeRecursively();
                        }
                    }
                } else {
                    // Removing or renaming failed
                    unprocessedSrcNames.push_back(namesToReplace[i]);
                }
            }
        }
    }

    // removing encrypted files from filesOperations list
    for (const auto& n : namesToReplace) {
        if (std::find(unprocessedSrcNames.begin(), unprocessedSrcNames.end(), n) == unprocessedSrcNames.end())
            removeFromFileOps(n, true);
    }

    waitDlg.hide();

    showResultMsg(unprocessedSrcNames);
}

void MainWindow::removeFromFileOps(const std::wstring& name, bool bySourcePath) {
    /*
    std::remove_if(fileOperations.begin(), fileOperations.end(), [=](const FileOperation& op) {
        const std::wstring opFileName = bySourcePath ? op.sourcePathName : op.destinationPathName;
        bool res = opFileName == name;
        if (res)
            syncDlg.remove(op);
        return res;
    });
    */


    for (std::size_t i = 0; i < fileOperations.size(); ++i) {
        const std::wstring opFileName = bySourcePath ? fileOperations[i].sourcePathName : fileOperations[i].destinationPathName;
        if (opFileName == name) {
            FileOperation tmp(std::move(fileOperations[i]));
            fileOperations.erase(fileOperations.begin() + i);
            syncDlg.remove(tmp);
            break;
        }
    }
}

void MainWindow::onDiscardAllFiles() {
    fileOperations.clear();
    syncDlg.clear();
}

void MainWindow::onFilesOpWipeSelected(std::vector<std::wstring>& unprocessedSrcNames) {
    waitDlg.init(0);
    waitDlg.show();
    std::vector<std::wstring> names;
    std::vector<std::wstring> relativePaths;
    for (const auto& op : fileOperations) {
        if (op.processItem) {
            names.emplace_back(op.destinationPathName);
            relativePaths.emplace_back(op.relativePath);
        }
    }
    //std::vector<std::string> unprocessedSrcNames;
    waitDlg.init(names.size());
    unprocessedSrcNames.clear();
    /*const int processedCount = */processItems(names, unprocessedSrcNames, L"/", &MainWindow::doRemove);

    // removing encrypted files from filesOperations list
    for (std::size_t i = 0; i < names.size(); ++i) {
        const std::wstring name = names[i];
        if (std::find(unprocessedSrcNames.begin(), unprocessedSrcNames.end(), name) == unprocessedSrcNames.end()) {
            removeFromFileOps(name, false);
            // Remove dirs if necessary

            const QString topLevelRelPath = getTopLevelRelPath(name, relativePaths[i]);
            if (topLevelRelPath.size() != 0) {
                std::vector<std::wstring> names, unprocessedSrcNames;
                names.push_back(topLevelRelPath.toStdWString());
                if (processItems(names, unprocessedSrcNames, L"/", &MainWindow::doCount) == 0) {
                    QDir d(topLevelRelPath);
                    d.removeRecursively();
                }
            }

            // remove dir
            /*
            QFileInfo fi(n.c_str());
            if (fi.exists() && fi.isDir()) {
                QDir d(n.c_str());
                d.removeRecursively();
            }
            */
        }
    }
    waitDlg.hide();
    showResultMsg(unprocessedSrcNames);
}

void MainWindow::onShowAbout() {
    AboutDialog d;

#ifndef VERSION
    std::string VERSION;
    QFile proFile(":/FileProtect.pro");
    if (proFile.open(QIODevice::ReadOnly)) {
        QByteArray line = proFile.readLine();
        while (line.size() > 0) {
            std::string tmp(line);
            if (tmp.find("VERSION = ") == 0) {
                size_t i = tmp.rfind("=");
                if (i != std::string::npos && i != tmp.size() - 1) {
                    tmp = tmp.substr(i + 1, tmp.size() - i);
                    if (tmp.size() > 0) {
                        // removing spaces
                        size_t t = tmp.find_first_not_of(" \t", 0);
                        if (t != std::string::npos)
                            tmp = tmp.substr(t, tmp.size() - t);

                        t = tmp.find_first_of(" \t\r\n", 0);
                        if (t != std::string::npos)
                            tmp = tmp.substr(0, t);
                        VERSION = tmp;
                    }
                }
            }
            line = proFile.readLine();
        }
        proFile.close();
    }
#endif
    d.setVersion(VERSION);

    d.exec();

}
