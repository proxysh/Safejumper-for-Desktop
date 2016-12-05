#ifndef DLG_NEWNODE_H
#define DLG_NEWNODE_H

#include <QDialog>

namespace Ui
{
class Dlg_newnode;
}

class Dlg_newnode : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_newnode(const QString & msg, QWidget *parent = 0);
    ~Dlg_newnode();

    bool IsCyclePort()
    {
        return _port;   // true - if cycle ports; false - cycle nodes
    }
private slots:
    void _ac_Cancel();
    void _ac_Confirm();
private:
    Ui::Dlg_newnode *ui;
    bool _port;
};

#endif // DLG_NEWNODE_H
