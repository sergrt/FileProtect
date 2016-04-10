#include "FilesSyncDialog.h"
#include "ui_FilesSyncDialog.h"

FilesSyncDialog::FilesSyncDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::FilesSyncDialog) {

    ui->setupUi(this);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    ui->tableWidget->insertColumn(0);
    QStringList header;
    header.push_back("#");
    header.push_back("File name");
    header.push_back("File size, bytes");
    header.push_back("DS");
    header.push_back("DT");
    ui->tableWidget->setHorizontalHeaderLabels(header);
}

FilesSyncDialog::~FilesSyncDialog() {
    delete ui;
}
void FilesSyncDialog::push_back(const FileOperation &op) {
    const int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    QTableWidgetItem* checkBox = new QTableWidgetItem();
    checkBox->setCheckState(Qt::Unchecked);
    ui->tableWidget->setItem(row, 0, checkBox);
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(op.sourcePathName.c_str()));
}
