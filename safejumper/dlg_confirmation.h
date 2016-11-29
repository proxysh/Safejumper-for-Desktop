#ifndef DLG_CONFIRMATION_H
#define DLG_CONFIRMATION_H

#include <QDialog>

namespace Ui
{
class Dlg_confirmation;
}

class Dlg_confirmation : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_confirmation(const QString & msg, QWidget *parent = 0);
    ~Dlg_confirmation();
private slots:
    void _ac_Cancel();
    void _ac_Confirm();
private:
    Ui::Dlg_confirmation *ui;
};

#endif // DLG_CONFIRMATION_H
