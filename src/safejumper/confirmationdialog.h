#ifndef DLG_CONFIRMATION_H
#define DLG_CONFIRMATION_H

#include <QDialog>

namespace Ui
{
class ConfirmationDialog;
}

class ConfirmationDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfirmationDialog(const QString & message, QWidget *parent = 0);
    ~ConfirmationDialog();
private slots:
    void on_cancelButton_clicked();
    void on_confirmButton_clicked();
private:
    Ui::ConfirmationDialog *ui;
};

#endif // DLG_CONFIRMATION_H
