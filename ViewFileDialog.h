#ifndef VIEWFILEDIALOG_H
#define VIEWFILEDIALOG_H

#include <QDialog>

namespace Ui {
    class ViewFileDialog;
}

class ViewFileDialog : public QDialog {
    Q_OBJECT

public:
    explicit ViewFileDialog(QWidget* parent = 0);
    ~ViewFileDialog();
    void setText(const QString& text);

private:
    Ui::ViewFileDialog* ui;
};

#endif // VIEWFILEDIALOG_H
