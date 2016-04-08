#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"

#include <QMessageBox>
/*
OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
}
*/
OptionsDialog::OptionsDialog(Options& options) :
    QDialog(), ui(new Ui::OptionsDialog), options(options)
{
    ui->setupUi(this);
    connect(ui->rbFile, &QRadioButton::toggled, this, [=](bool toggled){ui->frameKeyFile->setEnabled(toggled);});
    connect(ui->rbExt, &QRadioButton::toggled, this, [=](bool toggled){ui->frameWipeProgram->setEnabled(toggled);});

    connect(ui->bnCancel, &QPushButton::clicked, this, [=](){close();});
    connect(ui->bnOk, &QPushButton::clicked, this, &OptionsDialog::onOkClick);


    update();
    //ui->frameKeyFile->setEnabled(ui->rbFile->isChecked());
    //ui->frameWipeProgram->setEnabled(ui->rbExt->isChecked());
}
void OptionsDialog::update() {
    Options::KeyStorage k = options.keyStorage();
    ui->rbKbd->setChecked(k == Options::KeyStorage::Keyboard);
    ui->rbFile->setChecked(k == Options::KeyStorage::File);
    ui->rbFile->toggle();

    Options::WipeMethod w = options.wipeMethod();
    ui->rbNone->setChecked(w == Options::WipeMethod::Regular);
    ui->rbExt->setChecked(w == Options::WipeMethod::External);
    ui->rbExt->toggle();

    ui->leKeyFile->setText(options.keyFile().c_str());
    ui->leWipeProgram->setText(options.wipeProgram().c_str());
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::onOkClick() {
    if (options.validate()) {
        options.setKeyStorage(ui->rbKbd->isChecked() ? Options::KeyStorage::Keyboard : Options::KeyStorage::File);
        options.setKeyFile(ui->leKeyFile->text().toStdString());
        options.setWipeMethod(ui->rbNone->isChecked() ? Options::WipeMethod::Regular : Options::WipeMethod::External);
        options.setWipeProgram(ui->leWipeProgram->text().toStdString());
        options.save();
        close();
    } else {
        QMessageBox m;
        m.setIcon(QMessageBox::Warning);
        m.setText("Options are not complete");
        m.setWindowTitle("Validate result");
        m.exec();
    }
}
