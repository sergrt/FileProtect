#include "FilesSyncDialog.h"
#include "ui_FilesSyncDialog.h"
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>

const QString textDifY = "Y";
const QString textDifN = "N";

#include <QFontDatabase>

FilesSyncDialog::FilesSyncDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::FilesSyncDialog) {
    ui->setupUi(this);
    connect(ui->bnSelAll, &QPushButton::clicked, this, &FilesSyncDialog::onSelAllClick);
    connect(ui->bnSelAllDif, &QPushButton::clicked, this, &FilesSyncDialog::onSelAllDifClick);
    connect(ui->bnSelDifSize, &QPushButton::clicked, this, &FilesSyncDialog::onSelDifSizeClick);
    connect(ui->bnSelDifTime, &QPushButton::clicked, this, &FilesSyncDialog::onSelDifTimeClick);
    connect(ui->bnSelNone, &QPushButton::clicked, this, &FilesSyncDialog::onSelNoneClick);

    connect(ui->bnEncryptSel, &QPushButton::clicked, this, &FilesSyncDialog::onEncryptSelClick);
    connect(ui->bnWipeSel, &QPushButton::clicked, this, &FilesSyncDialog::onWipeSelClick);

    connect(ui->bnDiscard, &QPushButton::clicked, this, [=]() {
        emit discardAllFiles();
        QDialog::accept(); });
    connect(ui->bnBackToMain, &QPushButton::clicked, this, [=]() { /*QDialog::reject();*/QDialog::accept(); });

    ui->tableWidget->insertColumn(ColumnCheckBox);
    ui->tableWidget->insertColumn(ColumnSourceFileName);
    //ui->tableWidget->insertColumn(ColumnInitialFileSize);
    ui->tableWidget->insertColumn(ColumnDifSize);
    ui->tableWidget->insertColumn(ColumnDifTime);
    
    QStringList header;
    header.push_back("#");
    header.push_back("File name");
    //header.push_back("Src size, bytes");
    header.push_back("Dif Size");
    header.push_back("Dif Time");
    ui->tableWidget->setHorizontalHeaderLabels(header);

    const int w = this->width();
    ui->tableWidget->setColumnWidth(ColumnCheckBox,        0.04*w);
    ui->tableWidget->setColumnWidth(ColumnSourceFileName,  0.6*w);
    //ui->tableWidget->setColumnWidth(ColumnInitialFileSize, 0.2*w);
    ui->tableWidget->setColumnWidth(ColumnDifSize,         0.1*w);
    ui->tableWidget->setColumnWidth(ColumnDifTime,         0.1*w);

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    ui->tableWidget->setFont(fixedFont);
    QFontInfo fnt(ui->tableWidget->font());

    ui->tableWidget->verticalHeader()->sectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget->verticalHeader()->setDefaultSectionSize(fnt.pixelSize() + 6);
}

FilesSyncDialog::~FilesSyncDialog() {
    delete ui;
}

void FilesSyncDialog::push_back(const FileOperation &op) {
    const int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    // Checkbox
    QTableWidgetItem* checkBox = new QTableWidgetItem();
    checkBox->setCheckState(Qt::Unchecked);
    ui->tableWidget->setItem(row, ColumnCheckBox, checkBox);
    // File name
    ui->tableWidget->setItem(row, ColumnSourceFileName, new QTableWidgetItem(op.sourcePathName.c_str()));
    // File size
    /*
    QTableWidgetItem* sizeItem = new QTableWidgetItem(QString("%1").arg(op.initialFileSize));
    sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    ui->tableWidget->setItem(row, ColumnInitialFileSize, sizeItem);
    */
    // Dif size
    QTableWidgetItem* difSizeItem = new QTableWidgetItem();
    difSizeItem->setText(textDifN);
    difSizeItem->setData(Qt::UserRole, false);
    // Dif time
    QTableWidgetItem* difTimeItem = new QTableWidgetItem();
    difTimeItem->setText(textDifN);
    difTimeItem->setData(Qt::UserRole, false);

    bool difSize = false;
    bool difTime = false;
    QFile f(op.destinationPathName.c_str());
    if (f.exists()) {
        const unsigned long newSize = f.size();
        if (newSize != op.initialFileSize) {
            difSizeItem->setText(textDifY);
            difSizeItem->setData(Qt::UserRole, true);
            difSize = true;
        }

        QFileInfo fi(f);
        if (fi.lastModified().toTime_t() != op.initialModificationTime) {
            difTimeItem->setText(textDifY);
            difTimeItem->setData(Qt::UserRole, true);
            difTime = true;
        }
    }
    ui->tableWidget->setItem(row, ColumnDifSize, difSizeItem);
    ui->tableWidget->setItem(row, ColumnDifTime, difTimeItem);

    // Colorizing
    if (difSize || difTime) {
        QColor color;
        if (difSize && difTime)
            color = difAllColor;
        else if (difSize)
            color = difSizeColor;
        else
            color = difTimeColor;
        for (int i = 0; i < ui->tableWidget->columnCount(); ++i)
           ui->tableWidget->item(row, i)->setBackgroundColor(color);
    }
}

void FilesSyncDialog::clear() {
    while (ui->tableWidget->rowCount() > 0)
        ui->tableWidget->removeRow(0);
}

void FilesSyncDialog::remove(const FileOperation& op) {
    for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
        const std::string sName = ui->tableWidget->item(i, ColumnSourceFileName)->text().toStdString();
        if (sName == op.sourcePathName) {
            ui->tableWidget->removeRow(i);
            break;
        }
    }
}

void FilesSyncDialog::onSelAllClick() {
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row)
        ui->tableWidget->item(row, ColumnCheckBox)->setCheckState(Qt::Checked);
}

void FilesSyncDialog::onSelAllDifClick() {
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        const Qt::CheckState state = ui->tableWidget->item(row, ColumnDifSize)->data(Qt::UserRole).toBool()
                || ui->tableWidget->item(row, ColumnDifTime)->data(Qt::UserRole).toBool() ? Qt::Checked : Qt::Unchecked;

        ui->tableWidget->item(row, ColumnCheckBox)->setCheckState(state);
    }
}

void FilesSyncDialog::onSelDifSizeClick() {
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        const Qt::CheckState state = ui->tableWidget->item(row, ColumnDifSize)->data(Qt::UserRole).toBool() ? Qt::Checked : Qt::Unchecked;
        ui->tableWidget->item(row, ColumnCheckBox)->setCheckState(state);
    }
}

void FilesSyncDialog::onSelDifTimeClick() {
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        const Qt::CheckState state = ui->tableWidget->item(row, ColumnDifTime)->data(Qt::UserRole).toBool() ? Qt::Checked : Qt::Unchecked;
        ui->tableWidget->item(row, ColumnCheckBox)->setCheckState(state);
    }
}

void FilesSyncDialog::onSelNoneClick() {
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row)
        ui->tableWidget->item(row, ColumnCheckBox)->setCheckState(Qt::Unchecked);
}

int FilesSyncDialog::updateFileOperations() {
    int res = 0;
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        if (ui->tableWidget->item(row, ColumnCheckBox)->checkState() == Qt::Checked) {
            emit setMarkForProcess(ui->tableWidget->item(row, ColumnSourceFileName)->text().toStdString());
            ++res;
        }
    }
    return res;
}

void FilesSyncDialog::onEncryptSelClick() {
    if (updateFileOperations() > 0) {
        std::vector<std::string> unprocessedSrcNames;
        emit restoreEncryptedSelected(unprocessedSrcNames);
        showResultMsg(unprocessedSrcNames);
    } else {
        showNoFilesSelectedMsg();
    }
}

void FilesSyncDialog::onWipeSelClick() {
    if (updateFileOperations() > 0) {
        std::vector<std::string> unprocessedSrcNames;
        emit wipeSelected(unprocessedSrcNames);
        showResultMsg(unprocessedSrcNames);
    } else {
        showNoFilesSelectedMsg();
    }
}
void FilesSyncDialog::showNoFilesSelectedMsg() const {
    QMessageBox m;
    m.setText("No files selected");
    m.setIcon(QMessageBox::Information);
    m.setWindowTitle("Unable to proceed");
    m.exec();
}

void FilesSyncDialog::showResultMsg(const std::vector<std::string>& unprocessedSrcNames) const {
    QMessageBox m;
    m.setWindowTitle("Operation summary");
    QString msgText;

    if (unprocessedSrcNames.size() > 0) {
        msgText = QString("Error processing files. %1 file(s) left unprocessed").arg(unprocessedSrcNames.size());
        const size_t maxFilesToOutput = 10;
        const size_t maxCount = std::min(maxFilesToOutput, unprocessedSrcNames.size());
        for (std::size_t i = 0; i < maxCount; ++i)
            msgText += (std::string("\n\t") + unprocessedSrcNames[i]).c_str();

        if (maxCount < unprocessedSrcNames.size())
            msgText += "\n\t...";

        m.setIcon(QMessageBox::Warning);
        m.exec();
    } else {
        msgText = "Operation completed";
        m.setIcon(QMessageBox::Information);
    }
    m.setText(msgText);
    m.exec();
}
