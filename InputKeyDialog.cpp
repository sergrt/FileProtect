#include "InputKeyDialog.h"
#include "ui_InputKeyDialog.h"

InputKeyDialog::InputKeyDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::InputKeyDialog) {
    ui->setupUi(this);
    connect(ui->bnOk, &QPushButton::clicked, this, &InputKeyDialog::onOkClick);
    connect(ui->bnCancel, &QPushButton::clicked, this, [=]() { QDialog::reject(); });
}

InputKeyDialog::~InputKeyDialog() {
    delete ui;
}

int InputKeyDialog::exec() {
    ui->leKey->setFocus();
    return QDialog::exec();
}

void InputKeyDialog::onOkClick() {
    if (ui->cbRemember->isChecked())
        key = ui->leKey->text().toStdString();

    QDialog::accept();
}

bool InputKeyDialog::keyStored() const {
    return key.size() > 0;
}

std::string InputKeyDialog::getKey() const {
    return key.size() == 0 ? ui->leKey->text().toStdString() : key;
}

void InputKeyDialog::clearUi() {
    ui->leKey->setText("");
}
