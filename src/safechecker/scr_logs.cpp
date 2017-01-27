#include "scr_logs.h"

#include <QClipboard>

#include "ui_scr_logs.h"
#include "testdialog.h"
#include "common.h"
#include "wndmanager.h"
#include "fonthelper.h"
#include "loginwindow.h"

Scr_Logs::Scr_Logs(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scr_Logs)
    , _moving(false)
{
#ifdef Q_OS_WIN
    setWindowFlags(Qt::Dialog);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif

    ui->setupUi(this);
    this->setFixedSize(this->size());
#ifndef Q_OS_MAC
    FontHelper::SetFont(this);
#endif

//	QPoint p0 = _WndStart = pos();
//	WndManager::DoShape(this);
//	QPoint p1 = pos();
//		move(p0);
    qApp->installEventFilter(this);
}

void Scr_Logs::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

Scr_Logs::~Scr_Logs()
{
    delete ui;
}

std::auto_ptr<Scr_Logs> Scr_Logs::_inst;
Scr_Logs * Scr_Logs::Instance()
{
    if (!_inst.get())
        _inst.reset(new Scr_Logs());
    return _inst.get();
}

void Scr_Logs::ShowSupportUrl()
{
    OpenUrl_Support();
}

void Scr_Logs::ToScr_Connect()
{
    WndManager::Instance()->ToPrimary();
}

void Scr_Logs::Log(const QString & s)
{
    QTextCursor c = ui->e_Logs->textCursor();
    c.movePosition(QTextCursor::End);
    c.insertText(s);
    c.movePosition(QTextCursor::End);
    int n = ui->e_Logs->toPlainText().length();
    c.setPosition(n);
    ui->e_Logs->setTextCursor(c);
}

void Scr_Logs::Clicked_Copytoclipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(ui->e_Logs->toPlainText());
}

void Scr_Logs::keyPressEvent(QKeyEvent * e)
{
    if(e->key() != Qt::Key_Escape)
        QDialog::keyPressEvent(e);
}

bool Scr_Logs::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseMove: {
        if (_moving) {
            QPoint d = QCursor::pos() - _CursorStart;
            if (d.x() != 0 || d.y() != 0) {
                QPoint NewAbs = _WndStart + d;
                this->move(NewAbs);
            }
        }
        return false;
    }
    case QEvent::MouseButtonRelease: {
        _moving = false;
//			_WndStart = pos();
        return false;
    }
    default:
        return QDialog::eventFilter(obj, event);
    }
}

void Scr_Logs::Pressed_Head()
{
    _WndStart = this->pos();
    _CursorStart = QCursor::pos();
    _moving = true;
}

void Scr_Logs::Clicked_Min()
{
    WndManager::Instance()->HideThis(this);
}

void Scr_Logs::Clicked_Cross()
{
    LoginWindow::Instance()->quitApplication();
}
