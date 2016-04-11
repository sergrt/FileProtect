#include "FilesSyncDialog.h"
#include "ui_FilesSyncDialog.h"
#include <QFile>
#include <QFileInfo>
#include <QDateTime>

FilesSyncDialog::FilesSyncDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::FilesSyncDialog) {

    ui->setupUi(this);
    connect(ui->bnSelAll, &QPushButton::clicked, this, &FilesSyncDialog::onSelAllClick);
    connect(ui->bnSelAllDif, &QPushButton::clicked, this, &FilesSyncDialog::onSelAllDifClick);
    connect(ui->bnSelDifSize, &QPushButton::clicked, this, &FilesSyncDialog::onSelDifSizeClick);
    connect(ui->bnSelDifTime, &QPushButton::clicked, this, &FilesSyncDialog::onSelDifTimeClick);
    connect(ui->bnSelNone, &QPushButton::clicked, this, &FilesSyncDialog::onSelNoneClick);

    connect(ui->bnEncryptSel, &QPushButton::clicked, this, &FilesSyncDialog::onEncryptSelClick);


    ui->tableWidget->insertColumn(ColumnCheckBox);
    ui->tableWidget->insertColumn(ColumnSourceFileName);
    ui->tableWidget->insertColumn(ColumnInitialFileSize);
    ui->tableWidget->insertColumn(ColumnDifSize);
    ui->tableWidget->insertColumn(ColumnDifTime);
    QStringList header;
    header.push_back("#");
    header.push_back("File name");
    header.push_back("Src size, bytes");
    header.push_back("DS");
    header.push_back("DT");
    ui->tableWidget->setHorizontalHeaderLabels(header);

    //ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    const int w = this->width();

    ui->tableWidget->setColumnWidth(ColumnCheckBox,        0.1*w);
    ui->tableWidget->setColumnWidth(ColumnSourceFileName,  0.5*w);
    ui->tableWidget->setColumnWidth(ColumnInitialFileSize, 0.2*w);
    ui->tableWidget->setColumnWidth(ColumnDifSize,         0.1*w);
    ui->tableWidget->setColumnWidth(ColumnDifTime,         0.1*w);

    //ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

}

FilesSyncDialog::~FilesSyncDialog() {
    delete ui;
}
void FilesSyncDialog::push_back(const FileOperation &op) {
    const int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    QTableWidgetItem* checkBox = new QTableWidgetItem();
    checkBox->setCheckState(Qt::Unchecked);
    ui->tableWidget->setItem(row, ColumnCheckBox, checkBox);
    ui->tableWidget->setItem(row, ColumnSourceFileName, new QTableWidgetItem(op.sourcePathName.c_str()));
    QTableWidgetItem* sizeItem = new QTableWidgetItem(QString("%1").arg(op.initialFileSize));
    sizeItem->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
    ui->tableWidget->setItem(row, ColumnInitialFileSize, sizeItem);

    QTableWidgetItem* difSizeItem = new QTableWidgetItem();
    difSizeItem->setText("N");
    difSizeItem->setData(Qt::UserRole, false);
    QTableWidgetItem* difTimeItem = new QTableWidgetItem();
    difTimeItem->setText("N");
    difTimeItem->setData(Qt::UserRole, false);

    bool difSize = false;
    bool difTime = false;
    QFile f(op.destinationPathName.c_str());
    if (f.exists()) {
        const unsigned long newSize = f.size();
        if (newSize != op.initialFileSize) {
            difSizeItem->setText("Y");
            difSizeItem->setData(Qt::UserRole, true);
            difSize = true;
        }

        QFileInfo fi(f);
        if (fi.lastModified().toTime_t() != op.initialModificationTime) {
            difTimeItem->setText("Y");
            difTimeItem->setData(Qt::UserRole, true);
            difTime = true;
        }
    }
    ui->tableWidget->setItem(row, ColumnDifSize, difSizeItem);
    ui->tableWidget->setItem(row, ColumnDifTime, difTimeItem);

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

void FilesSyncDialog::onEncryptSelClick() {
    const int rowCount = ui->tableWidget->rowCount();
    for (int row = 0; row < rowCount; ++row) {
        if (ui->tableWidget->item(row, ColumnCheckBox)->checkState() == Qt::Checked) {
            emit setRestoreEncrypted(ui->tableWidget->item(row, ColumnSourceFileName)->text().toStdString());
        }
    }
    emit restoreEncrypted();
}
