#ifndef SCR_TABLE_H
#define SCR_TABLE_H

#include <memory>

#include <QDialog>

#include "common.h"

namespace Ui
{
class Scr_Table;
}

enum ServerNodeState {
    snsDown = 0,
    snsConnected,
    snsTimeout,
    snsHttpFail
};

class Scr_Table : public QDialog
{
    Q_OBJECT

public:
    explicit Scr_Table(QWidget *parent = 0);
    ~Scr_Table();

    static Scr_Table * Instance();
    static bool IsExists()
    {
        return (_inst.get() != NULL);
    }
    static void Cleanup()
    {
        if (_inst.get() != NULL) delete _inst.release();
    }

    void SetStatus(int enc, size_t srv, int port, ServerNodeState st);
    void SetStatus(int enc, size_t srv, int port, const QString & custom);

private:
    Ui::Scr_Table *ui;
    static std::auto_ptr<Scr_Table> _inst;
    void Fill();
};


#endif // SCR_TABLE_H
