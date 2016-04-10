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
    connect(ui->rbFolder, &QRadioButton::toggled, this, [=](bool toggled){ui->frameDecryptFolder->setEnabled(toggled);});

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
    ui->frameKeyFile->setEnabled(k == Options::KeyStorage::File);

    Options::WipeMethod w = options.wipeMethod();
    ui->rbNone->setChecked(w == Options::WipeMethod::Regular);
    ui->rbExt->setChecked(w == Options::WipeMethod::External);
    ui->frameWipeProgram->setEnabled(w == Options::WipeMethod::External);

    Options::DecryptionPlace d = options.decryptionPlace();
    ui->rbSame->setChecked(d == Options::DecryptionPlace::Inplace);
    ui->rbFolder->setChecked(d == Options::DecryptionPlace::Specified);
    ui->frameDecryptFolder->setEnabled(d == Options::DecryptionPlace::Specified);

    ui->leKeyFile->setText(options.keyFile().c_str());
    ui->leWipeProgram->setText(options.wipeProgram().c_str());
    ui->leDecryptFolder->setText(options.decryptionFolder().c_str());
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::onOkClick() {
    /*if (options.validate()) */{
        options.setKeyStorage(ui->rbKbd->isChecked() ? Options::KeyStorage::Keyboard : Options::KeyStorage::File);
        options.setKeyFile(ui->leKeyFile->text().toStdString());
        options.setWipeMethod(ui->rbNone->isChecked() ? Options::WipeMethod::Regular : Options::WipeMethod::External);
        options.setWipeProgram(ui->leWipeProgram->text().toStdString());
        options.setDecryptionPlace(ui->rbSame->isChecked() ? Options::DecryptionPlace::Inplace : Options::DecryptionPlace::Specified);
        options.setDecryptionFolder(ui->leDecryptFolder->text().toStdString());
        options.save();
        close();
    }/* else {
        QMessageBox m;
        m.setIcon(QMessageBox::Warning);
        m.setText("Options are not complete");
        m.setWindowTitle("Validate result");
        m.exec();
    }*/
}
