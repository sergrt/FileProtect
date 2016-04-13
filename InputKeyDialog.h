#ifndef INPUTKEYDIALOG_H
#define INPUTKEYDIALOG_H

#include <QDialog>

namespace Ui {
    class InputKeyDialog;
}

class InputKeyDialog : public QDialog {
    Q_OBJECT

public:
    explicit InputKeyDialog(QWidget *parent = 0);
    ~InputKeyDialog();
    std::string getKey() const;
private:
    Ui::InputKeyDialog *ui;
};

#endif // INPUTKEYDIALOG_H
