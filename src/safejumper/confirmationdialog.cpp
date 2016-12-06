#include "confirmationdialog.h"

#include "ui_confirmationdialog.h"
#include "wndmanager.h"
#include "fonthelper.h"

ConfirmationDialog::ConfirmationDialog(const QString & message, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfirmationDialog)
{
    ui->setupUi(this);
#ifndef Q_OS_MAC
    FontHelper::SetFont(this);
#endif
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    WndManager::DoShape(this);

    this->setAttribute(Qt::WA_MacNoShadow, false);
    ui->messageLabel->setText(message);
}

ConfirmationDialog::~ConfirmationDialog()
{
    delete ui;
}

void ConfirmationDialog::on_cancelButton_clicked()
{
    QDialog::reject();
}

void ConfirmationDialog::on_confirmButton_clicked()
{
    QDialog::accept();
}
