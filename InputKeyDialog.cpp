#include "InputKeyDialog.h"
#include "ui_InputKeyDialog.h"

InputKeyDialog::InputKeyDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::InputKeyDialog) {
    ui->setupUi(this);
}

InputKeyDialog::~InputKeyDialog() {
    delete ui;
}

std::string InputKeyDialog::getKey() const {
    return ui->leKey->text().toStdString();
}
