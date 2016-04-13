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

const int dirFilter = QDir::Files | QDir::Dirs | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot;

MainWindow::MainWindow(QWidget *parent) :
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

    const std::string rootPath = options.rootPath();
    model->setRootPath(rootPath.size() > 0 ? rootPath.c_str() : QDir::currentPath());

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
    new QShortcut(QKeySequence(Qt::Key_F1), this, SLOT(onEncryptSelected()));
    new QShortcut(QKeySequence(Qt::Key_F2), this, SLOT(onDecryptSelected()));
    //new QShortcut(QKeySequence(Qt::Key_F3), this, SLOT(onViewSelected()));
    new QShortcut(QKeySequence(Qt::Key_F4), this, SLOT(onExecuteSelected()));
    new QShortcut(QKeySequence(Qt::Key_F5), this, SLOT(onShowDecrypted()));
    new QShortcut(QKeySequence(Qt::Key_F6), this, SLOT(onSetAsRoot()));
    new QShortcut(QKeySequence(Qt::Key_F8), this, SLOT(onWipeSelected()));
    new QShortcut(QKeySequence(Qt::Key_F9), this, SLOT(onOptionsClick()));
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp), this, SLOT(onUpOneLevelClick()));
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

void MainWindow::onExecuteSelected() {
    QDesktopServices::openUrl(QUrl(model()->filePath(ui->treeView->currentIndex())));
}

void MainWindow::onShowDecrypted() {
    syncDlg.clear();
    for (const auto& op : fileOperations)
        syncDlg.push_back(op);
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
    options.setRootPath(newPath.toStdString());
    options.save();
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
    QMessageBox m;
    m.setText("You are about to wipe file(s). Note that this is potentially irreversible operation. Are you sure you want to proceed?");
    m.setWindowTitle("Confirmation");
    m.setIcon(QMessageBox::Warning);
    m.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    m.setDefaultButton(QMessageBox::No);
    if (m.exec() == QMessageBox::Yes)
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

        removeFromFileOps(infile, true);
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

bool MainWindow::doCount(const std::string&/* fileName*/) {
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
    if (res)
        removeFromFileOps(fileName, true);

    return res;
}

int MainWindow::processItems(const std::vector<std::string>& names, std::vector<std::string>& unprocessedSrcNames, bool (MainWindow::*procFunc)(const std::string&)) {
    int res = 0;
    for (const auto& name : names) {
        const char* tmpName = name.c_str();
        QFileInfo f(tmpName);
        if (!f.isDir()) {
            waitDlg.step();
            qApp->processEvents();

            if ((this->*procFunc)(f.absoluteFilePath().toStdString()))
                ++res;
            else
                unprocessedSrcNames.push_back(name);
        } else {
            QDir dir(tmpName);
            QFileInfoList fil = dir.entryInfoList(QDir::Filter(dirFilter));
            for (const auto& f : fil) {
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

        if (syncDlg.exec() == QDialog::Rejected)
            event->ignore();
    }
}
void MainWindow::onMarkForProcess(const std::string& encryptedName) {
    for (auto& op : fileOperations) {
        if (encryptedName == op.sourcePathName)
            op.processItem = true;
    }
}

void MainWindow::onFilesOpEncryptedSelected(std::vector<std::string>& unprocessedSrcNames) {
    std::vector<std::string> names;
    std::vector<std::string> namesToReplace;
    for (const auto& op : fileOperations) {
        if (op.processItem) {
            names.emplace_back(op.destinationPathName);
            namesToReplace.emplace_back(op.sourcePathName);
        }
    }
    //std::vector<std::string> unprocessedSrcNames;
    unprocessedSrcNames.clear();
    /*int processedCount = */processItems(names, unprocessedSrcNames, &MainWindow::doEncrypt);

    // removing encrypted files from filesOperations list
    for (const auto& n : namesToReplace) {
        if (std::find(unprocessedSrcNames.begin(), unprocessedSrcNames.end(), n) == unprocessedSrcNames.end())
            removeFromFileOps(n, true);
    }

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
                QFile f(names[i].c_str());
                QFile d(namesToReplace[i].c_str());
                if (d.remove() && QFile::rename(names[i].c_str(), namesToReplace[i].c_str()))
                    --filesLeft;
                else
                    unprocessedSrcNames.push_back(namesToReplace[i]);
            }
        }
    }
}

void MainWindow::removeFromFileOps(const std::string& name, bool bySourcePath) {

    for (std::size_t i = 0; i < fileOperations.size(); ++i) {
        const std::string opFileName = bySourcePath ? fileOperations[i].sourcePathName : fileOperations[i].destinationPathName;
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

void MainWindow::onFilesOpWipeSelected(std::vector<std::string>& unprocessedSrcNames) {
    std::vector<std::string> names;
    for (const auto& op : fileOperations) {
        if (op.processItem)
            names.emplace_back(op.destinationPathName);
    }
    //std::vector<std::string> unprocessedSrcNames;
    unprocessedSrcNames.clear();
    /*const int processedCount = */processItems(names, unprocessedSrcNames, &MainWindow::doRemove);

    // removing encrypted files from filesOperations list
    for (const auto& n : names) {
        if (std::find(unprocessedSrcNames.begin(), unprocessedSrcNames.end(), n) == unprocessedSrcNames.end())
            removeFromFileOps(n, false);
    }
}

