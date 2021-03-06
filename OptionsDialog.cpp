#include "OptionsDialog.h"
#include "ui_OptionsDialog.h"
#include <QMessageBox>
#include <QFileDialog>

OptionsDialog::OptionsDialog(Options& options)
    : QDialog(), ui(new Ui::OptionsDialog), options(options) {
    ui->setupUi(this);
    connect(ui->rbFile, &QRadioButton::toggled, this, [=](bool toggled){ui->frameKeyFile->setEnabled(toggled);});
    connect(ui->rbExt, &QRadioButton::toggled, this, [=](bool toggled){ui->frameWipeProgram->setEnabled(toggled);});
    connect(ui->rbFolder, &QRadioButton::toggled, this, [=](bool toggled){ui->frameDecryptFolder->setEnabled(toggled);});

    connect(ui->bnBrowseKey, &QPushButton::clicked, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Key file"), nullptr, nullptr, nullptr, QFileDialog::DontResolveSymlinks);
        if (fileName.length() > 0)
            ui->leKeyFile->setText(fileName);
    });
    connect(ui->bnBrowseWipingProg, &QPushButton::clicked, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Wiping program"), nullptr, nullptr, nullptr, QFileDialog::DontResolveSymlinks);
        if (fileName.length() > 0)
            ui->leWipeProgram->setText(fileName);
    });
    connect(ui->bnBrowseDercyptFolder, &QPushButton::clicked, [=]() {
        QString dir = QFileDialog::getExistingDirectory(this, tr("Decryption folder"), nullptr, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (dir.length() > 0)
            ui->leDecryptFolder->setText(dir);
    });

    connect(ui->bnCancel, &QPushButton::clicked, this, [=](){close();});
    connect(ui->bnOk, &QPushButton::clicked, this, &OptionsDialog::onOkClick);

    update();
}

void OptionsDialog::update() {
    const Options::KeyStorage k = options.keyStorage();
    ui->rbKbd->setChecked(k == Options::KeyStorage::Keyboard);
    ui->rbFile->setChecked(k == Options::KeyStorage::File);
    ui->frameKeyFile->setEnabled(k == Options::KeyStorage::File);

    const Options::WipeMethod w = options.wipeMethod();
    ui->rbNone->setChecked(w == Options::WipeMethod::Regular);
    ui->rbExt->setChecked(w == Options::WipeMethod::External);
    ui->frameWipeProgram->setEnabled(w == Options::WipeMethod::External);

    const Options::DecryptionPlace d = options.decryptionPlace();
    ui->rbSame->setChecked(d == Options::DecryptionPlace::Inplace);
    ui->rbFolder->setChecked(d == Options::DecryptionPlace::Specified);
    ui->frameDecryptFolder->setEnabled(d == Options::DecryptionPlace::Specified);

    ui->leKeyFile->setText(QString::fromStdWString(options.keyFile()));
    ui->leWipeProgram->setText(QString::fromStdWString(options.wipeProgram()));
    ui->leDecryptFolder->setText(QString::fromStdWString(options.decryptionFolder()));
}

OptionsDialog::~OptionsDialog() {
    delete ui;
}

void OptionsDialog::onOkClick() {
    try {
        if (!validate())
            throw std::logic_error("Options not consistent");

        options.setKeyStorage(ui->rbKbd->isChecked() ? Options::KeyStorage::Keyboard : Options::KeyStorage::File);
        options.setKeyFile(ui->leKeyFile->text().toStdWString());
        options.setWipeMethod(ui->rbNone->isChecked() ? Options::WipeMethod::Regular : Options::WipeMethod::External);
        options.setWipeProgram(ui->leWipeProgram->text().toStdWString());
        options.setDecryptionPlace(ui->rbSame->isChecked() ? Options::DecryptionPlace::Inplace : Options::DecryptionPlace::Specified);
        options.setDecryptionFolder(ui->leDecryptFolder->text().toStdWString());
        if (!options.save())
            throw std::runtime_error("Unable to save options");
        close();
    } catch (std::exception& e) {
        QMessageBox m;
        m.setIcon(QMessageBox::Critical);
        m.setText((std::string("Save options failed: ") + std::string(e.what())).c_str());
        m.setWindowTitle("Error");
        m.exec();
    }
}

bool OptionsDialog::validate() const {
    bool res =
              (ui->rbKbd->isChecked()  || (/*ui->rbFile->isChecked()   && */ui->leKeyFile->text().size()       > 0))
           && (ui->rbNone->isChecked() || (/*ui->rbExt->isChecked()    && */ui->leWipeProgram->text().size()   > 0))
           && (ui->rbSame->isChecked() || (/*ui->rbFolder->isChecked() && */ui->leDecryptFolder->text().size() > 0));

    return res;
}
