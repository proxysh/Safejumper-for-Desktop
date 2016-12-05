#ifndef WNDMANAGER_H
#define WNDMANAGER_H

#include <QWidget>
#include <QDialog>
#include <memory>

#include "dlg_confirmation.h"
#include "dlg_newnode.h"

//  WndManager::Instance()->ToPrimary(this);
//  WndManager::Instance()->HideThis(this);
//  WndManager::DoShape(this);
class WndManager
{
public:
    static WndManager * Instance();
    ~WndManager();
    static void Cleanup()
    {
        if (_inst.get() != NULL) delete _inst.release();
    }

    QWidget * Primary();
    void ToPrimary();
    void ToSettings();
    void ToLogs();
    void ToMap();

    void CloseAll();
    void HideThis(QWidget * scr);

    void trans(const QPoint & p, QWidget * to);
    QPoint CurrPos();
    void SavePos();
    QWidget * ScrVisible();	 // NULL if no visible

    static void DoShape(QWidget * d);
    void HandleConnecting();
    void HandleConnected();
    void HandleDisconnected();

    void HandleState(const QString & word);		// just output it into header for debug purpose

    void ErrMsg(const QString & msg);
    void ToFront(QWidget * w);
    void ToFront();

    int Confirmation(const QString & msg);		// switch to primary, return dialog code: QDialog::Accepted or another

    void ShowPortDlg();
    void ClosePortDlg();
    bool IsCyclePort();		// true - if cycle ports; false - cycle nodes

    void ShowTable();
private:
    WndManager();
    static std::auto_ptr<WndManager> _inst;
    void trans(QWidget * from, QWidget * to);

    void SaveCoords(QWidget * from);
    int _x;
    int _y;
    void ApplyCoords(QWidget * to);

    void ToLogin();
    void ToConnect();

    void SaveAndHide(QWidget * from);
    void DoTrans(QWidget * to);
    Dlg_newnode * _DlgPort;
};

#endif // WNDMANAGER_H
