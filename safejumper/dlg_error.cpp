#include "dlg_error.h"

#include <QBitmap>
#include <QPainter>

#include "ui_dlg_error.h"
#include "common.h"
#include "wndmanager.h"
#include "fonthelper.h"

Dlg_Error::Dlg_Error(const QString & msg, const QString & caption, QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Dlg_Error)
{
	ui->setupUi(this);
#ifndef Q_OS_MAC
	FontHelper::SetFont(this);
#endif
	WndManager::DoShape(this);
	this->setAttribute(Qt::WA_MacNoShadow, false);
	ui->L_Caption->setText(caption);
	ui->L_Text->setText(msg);
}

Dlg_Error::~Dlg_Error()
{
	delete ui;
}

void Dlg_Error::resizeEvent(QResizeEvent * )
{
	// on resize: it harms shape
	//WndManager::DoShape(this);
}

void Dlg_Error::_ac_Close()
{
	DoClose();
}

void Dlg_Error::_ac_Support()
{
	OpenUrl_Support();
	DoClose();
}

void Dlg_Error::DoClose()
{
	this->accept();
}
