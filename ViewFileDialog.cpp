#include "ViewFileDialog.h"
#include "ui_ViewFileDialog.h"

ViewFileDialog::ViewFileDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ViewFileDialog) {
    ui->setupUi(this);
}

ViewFileDialog::~ViewFileDialog() {
    delete ui;
}

void ViewFileDialog::setText(const QString& text) {
    ui->textEdit->setText(text);
}
