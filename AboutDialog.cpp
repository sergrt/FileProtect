#include "AboutDialog.h"
#include "ui_AboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::AboutDialog) {
    ui->setupUi(this);
    connect(ui->bnOk, &QPushButton::clicked, this, [=]() {QDialog::accept();});
}

AboutDialog::~AboutDialog() {
    delete ui;
}

void AboutDialog::setVersion(const std::string& str) {
    ui->labelVer->setText(("v" + str).c_str());
}
