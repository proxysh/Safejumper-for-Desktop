#include "scr_logs.h"

#include <QClipboard>

#include "ui_scr_logs.h"
#include "scr_connect.h"
#include "common.h"
#include "wndmanager.h"
#include "fonthelper.h"

Scr_Logs::Scr_Logs(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Scr_Logs)
{
	ui->setupUi(this);
	this->setFixedSize(this->size());
#ifndef Q_OS_MAC
	FontHelper::SetFont(this);
#endif
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

