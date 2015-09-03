#include "dlg_confirmation.h"

#include "ui_dlg_confirmation.h"
#include "wndmanager.h"
#include "fonthelper.h"

Dlg_confirmation::Dlg_confirmation(const QString & msg, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dlg_confirmation)
{
	ui->setupUi(this);
#ifndef Q_OS_MAC
	FontHelper::SetFont(this);
#endif
	WndManager::DoShape(this);
	
	this->setAttribute(Qt::WA_MacNoShadow, false);
	ui->L_Text->setText(msg);
}

Dlg_confirmation::~Dlg_confirmation()
{
	delete ui;
}

void Dlg_confirmation::_ac_Cancel()
{
	this->reject();
}

void Dlg_confirmation::_ac_Confirm()
{
	this->accept();
}
