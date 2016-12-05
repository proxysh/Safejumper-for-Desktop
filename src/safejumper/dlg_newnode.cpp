#include "dlg_newnode.h"
#include "ui_dlg_newnode.h"
#include "wndmanager.h"
#include "fonthelper.h"

Dlg_newnode::Dlg_newnode(const QString & msg, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_newnode)
{
    ui->setupUi(this);
#ifndef Q_OS_MAC
    FontHelper::SetFont(this);
#endif
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    WndManager::DoShape(this);

    this->setAttribute(Qt::WA_MacNoShadow, false);
    ui->L_Text->setText(msg);
}

Dlg_newnode::~Dlg_newnode()
{
    delete ui;
}

void Dlg_newnode::_ac_Cancel()
{
    _port = false;
    this->accept();
}

void Dlg_newnode::_ac_Confirm()
{
    _port = true;
    this->accept();
}
