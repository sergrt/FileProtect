#include "WaitDialog.h"
#include "ui_WaitDialog.h"

WaitDialog::WaitDialog(QWidget *parent) :
    QDialog(parent), ui(new Ui::WaitDialog) {
    ui->setupUi(this);
}

WaitDialog::~WaitDialog() {
    delete ui;
}

void WaitDialog::step() {
    ui->progressBar->setValue(ui->progressBar->value() + 1);
}

void WaitDialog::init(const int m) {
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(m);
}
