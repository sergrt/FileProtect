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
    bool keyStored() const;
    void clearUi();
private:
    Ui::InputKeyDialog *ui;
    std::string key;
    void onOkClick();
};

#endif // INPUTKEYDIALOG_H
