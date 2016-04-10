#ifndef FILESSYNCDIALOG_H
#define FILESSYNCDIALOG_H

#include <QDialog>
#include "FileOperation.h"

namespace Ui {
    class FilesSyncDialog;
}

class FilesSyncDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilesSyncDialog(QWidget *parent = 0);
    ~FilesSyncDialog();

    void push_back(const FileOperation& op);
private:
    Ui::FilesSyncDialog *ui;
};

#endif // FILESSYNCDIALOG_H
