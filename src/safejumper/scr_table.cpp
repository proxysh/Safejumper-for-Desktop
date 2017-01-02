#include "scr_table.h"
#include "ui_scr_table.h"

#include "setting.h"
#include "authmanager.h"
#include "log.h"

Scr_Table::Scr_Table(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Scr_Table)
{
    ui->setupUi(this);
    Fill();
}

Scr_Table::~Scr_Table()
{
    delete ui;
}

std::auto_ptr<Scr_Table> Scr_Table::_inst;
Scr_Table * Scr_Table::Instance()
{
    if (!_inst.get())
        _inst.reset(new Scr_Table());
    return _inst.get();
}

void Scr_Table::Fill()
{
    // TODO: -1 encription

    //for (int enc = 0; enc < ENCRYPTION_COUNT; ++enc)
    const char * et = Setting::EncText(Setting::Encryption());
    this->setWindowTitle(et);	// TODO: -1 encryptions

    const std::vector<QString> & pstrs = Setting::Instance()->GetAllProt();
    const std::vector<int> & nums = Setting::Instance()->GetAllPorts();
    ui->tableWidget->setColumnCount(pstrs.size());
    QStringList head;
    for (size_t j = 0; j < pstrs.size(); ++j) {
        QString s;
        if (pstrs.at(j).contains("udp", Qt::CaseInsensitive))
            s = "udp ";
        else
            s = "tcp ";
        s += QString::number(nums.at(j));
        head.append(s);
    }

    // headline
    ui->tableWidget->setHorizontalHeaderLabels(head);

    const std::vector<size_t> & srvs = AuthManager::Instance()->currentEncryptionServers();
    ui->tableWidget->setRowCount(srvs.size());
    ui->tableWidget->verticalHeader()->setVisible(true);
    QStringList left;
    for (size_t k = 0; k < srvs.size(); ++k) {
        AServer sr = AuthManager::Instance()->GetSrv(srvs.at(k));
        left.append(sr.name + " " + sr.address);
    }
    ui->tableWidget->setVerticalHeaderLabels(left);
}

void Scr_Table::SetStatus(int enc, size_t srv, int port, ServerNodeState st)
{
    QString s;
    switch (st) {
    case snsDown: {
        s = "down";
        break;
    }
    case snsConnected: {
        s = "connected";
        break;
    }
    case snsTimeout: {
        s = "timeout";
        break;
    }
    case snsHttpFail: {
        s = "http fail";
        break;
    }
    default:
        s = "unknown";
    }
    SetStatus(enc, srv, port, s);
}

void Scr_Table::SetStatus(int enc, size_t srv, int port, const QString & custom)
{
    // TODO: -1 encription
    // srv id -->> line
    int row = -1;
    const std::vector<size_t> & srvs = AuthManager::Instance()->currentEncryptionServers();
    for (int k = 0; k < srvs.size(); ++k)
        if (srv == srvs.at(k))
            row = k;
    if (row < 0) {
        log::logt("cannot find server " + QString::number(srv));
        row = 0;
    }
    // port id
    int col = port;
    if (col < 0)
        col = 0;
    ui->tableWidget->setItem(row, col, new QTableWidgetItem(custom));
}
