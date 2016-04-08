#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "Options.h"

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    //explicit OptionsDialog(QWidget *parent = 0);
    OptionsDialog(Options& options);
    ~OptionsDialog();

private:
    Ui::OptionsDialog *ui;
    Options& options;

    void onOkClick();
    void update();
};

#endif // OPTIONSDIALOG_H
