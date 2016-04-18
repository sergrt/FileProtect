#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include "Options.h"

namespace Ui {
    class OptionsDialog;
}

class OptionsDialog : public QDialog {
    Q_OBJECT

public:
    OptionsDialog(Options& options);
    ~OptionsDialog();

private:
    Ui::OptionsDialog* ui;
    Options& options;

    void onOkClick();
    void update();
    bool validate() const;
};

#endif // OPTIONSDIALOG_H
