#include "sjmainwindow.h"

#include <QMenu>
#include <QFontDatabase>

#include "ui_sjmainwindow.h"
#include "scr_connect.h"
#include "scr_settings.h"
#include "scr_logs.h"
#include "scr_map.h"
#include "dlg_error.h"
#include "confirmationdialog.h"

#ifdef MONITOR_TOOL
#include "scr_table.h"
#endif	// MONITOR_TOOL

#include "authmanager.h"
#include "wndmanager.h"
#include "common.h"
#include "version.h"
#include "setting.h"

#include "openvpnmanager.h"

#include "osspecific.h"
#include "log.h"
#include "fonthelper.h"

SjMainWindow::SjMainWindow(QWidget *parent) :
    //QDialog(parent),
    QMainWindow(parent),
    ui(new Ui::SjMainWindow)
    , _ConnectAfterLogin(false)
    , _fixed(false)
    , _activatedcount(0)
    , _wifi_processing(false)
{
    ui->setupUi(this);
#ifdef Q_OS_WIN
    setWindowFlags(Qt::Dialog);
    setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
#endif
    setFixedSize(this->size());

    ui->cancelButton->hide();

#ifndef Q_OS_MAC
    FontHelper::SetFont(this);
    ui->eLogin->setFont(FontHelper::pt(14));
    ui->ePsw->setFont(FontHelper::pt(14));
#endif

    ui->eLogin->setAttribute(Qt::WA_MacShowFocusRect, 0);
    ui->ePsw->setAttribute(Qt::WA_MacShowFocusRect, 0);

    ui->L_Version->setText(QString("v ") + SJ_VERSION + ", build " + QString::number(SJ_BUILD_NUM) );

    CreateTrayIcon();
    _TrayIcon->show();

    {
        QSettings settings;
        if (settings.contains("pos")) {
            QPoint p = settings.value("pos").toPoint();
            WndManager::Instance()->trans(p, this);
        }
//		_WndStart = pos();
        ui->rememberMeButton->setChecked(settings.value("cb_Rememberme", true).toBool());

        if (ui->rememberMeButton->isChecked()) {
            if (settings.contains("eLogin"))
                ui->eLogin->setText(settings.value("eLogin", "").toString());
            if (settings.contains("ePsw"))
                ui->ePsw->setText(settings.value("ePsw", "").toString());
        }
    }

    StatusDisconnected();

    if (!ui->eLogin->text().isEmpty() && ui->ePsw->text().isEmpty())
        ui->ePsw->setFocus();
    else
        ui->eLogin->setFocus();
    DisableButtonsOnLogout();

//	WndManager::DoShape(this);

    QTimer::singleShot(210, this, SLOT(Timer_Constructed()));
}

void SjMainWindow::DisableButtonsOnLogout()
{
    _ac_Logout->setEnabled(false);	//_ac_Logout->setIcon(QIcon(":/icons-tm/close-grey.png"));
    _ac_Jump->setEnabled(false);	//_ac_Jump->setIcon(QIcon(":/icons-tm/jump-grey.png"));
    _ac_SwitchCountry->setEnabled(false);	//_ac_SwitchCountry->setIcon(QIcon(":/icons-tm/country-grey.png"));
}

void SjMainWindow::EnableButtonsOnLogin()
{
    _ac_Logout->setEnabled(true);	//_ac_Logout->setIcon(QIcon(":/icons-tm/close-red.png"));
    _ac_Jump->setEnabled(true);		//_ac_Jump->setIcon(QIcon(":/icons-tm/jump-red.png"));
    _ac_SwitchCountry->setEnabled(true);	//_ac_SwitchCountry->setIcon(QIcon(":/icons-tm/country-red.png"));
}

void SjMainWindow::on_rememberMeButton_toggled()
{
    QSettings settings;
    settings.setValue("cb_Rememberme", ui->rememberMeButton->isChecked());
}

void SjMainWindow::Timer_Constructed()
{
#ifndef Q_OS_OSX
    connect(g_pTheApp, SIGNAL(showUp()), this, SLOT(ToScr_Primary()));
#endif

    Scr_Logs * l = Scr_Logs::Instance();

    if (l->IsExists())		// force construction
        if (OpenvpnManager::Instance()->openvpnRunning())
            OpenvpnManager::Instance()->killRunningOpenvpn();

    AuthManager::Instance()->DetermineOldIp();

    if (Setting::Instance()->IsCheckForUpdates()) {
        AuthManager::Instance()->StartDwnl_Updates();
    }

    if (Setting::Instance()->IsAutoconnect()) {
        _ConnectAfterLogin = true;
        DoLogin();
    }

    if (Setting::Instance()->IsInsecureWifi()) {
        StartWifiWatcher();
    }
}

SjMainWindow::~SjMainWindow()
{
    if (this->isVisible())
        WndManager::Instance()->HideThis(this);
    else
        WndManager::Instance()->CloseAll();
    if (this->isVisible())
        WndManager::Instance()->HideThis(this);
    WndManager::Instance()->SavePos();
    on_rememberMeButton_toggled(); // Save remember me setting
    SaveCreds();

    ClearConnecttoMenu();
    if (_ct_menu.get()) {
        if (_TrayMenu.get()) {
// TODO: -2			_TrayMenu->removeAction(_ct_menu.get());
        }
    }

    AuthManager::Cleanup();
    OpenvpnManager::cleanup();
    OsSpecific::Cleanup();
    Scr_Connect::Cleanup();
    Scr_Logs::Cleanup();
    Scr_Map::Cleanup();
    Scr_Settings::Cleanup();
    Setting::Cleanup();
    WndManager::Cleanup();
#ifdef MONITOR_TOOL
    Scr_Table::Cleanup();
#endif	// MONITOR_TOOL

    delete ui;
}

void SjMainWindow::SaveCreds()
{
    QSettings settings;
    if (ui->rememberMeButton->isChecked()) {
        settings.setValue("eLogin", ui->eLogin->text());
        settings.setValue("ePsw", ui->ePsw->text());	// TODO: -0
    }
}

void SjMainWindow::closeEvent(QCloseEvent * event)
{
    event->ignore();
    WndManager::Instance()->HideThis(this);
}

void SjMainWindow::CreateTrayIcon()
{
    CreateTrayMenu();
    _TrayIcon.reset(new QSystemTrayIcon(this));
    QMenu *menu = _TrayMenu.get();
    _TrayIcon->setContextMenu(menu);
    QIcon icon(OsSpecific::Instance()->IconDisconnected());
    _TrayIcon->setIcon(icon);
    connect(_TrayIcon.get(), SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(Icon_Activated(QSystemTrayIcon::ActivationReason)));
}

void SjMainWindow::CreateTrayMenu()
{
    _TrayMenu.reset(new QMenu(this));
    CreateActions();

    _TrayMenu->addAction(_ac_Connect.get());
    _TrayMenu->addAction(_ac_ConnectTo.get());
    _TrayMenu->addAction(_ac_Disconnect.get());
    _TrayMenu->addSeparator();

    _TrayMenu->addAction(_ac_Status.get());
    _TrayMenu->addSeparator();

    _TrayMenu->addAction(_ac_Jump.get());
    _TrayMenu->addAction(_ac_SwitchCountry.get());
    _ac_SwitchCountry->setEnabled(false);
    _TrayMenu->addSeparator();

    _TrayMenu->addAction(_ac_Settings.get());
    _TrayMenu->addAction(_ac_Logs.get());
    _TrayMenu->addSeparator();

    _TrayMenu->addAction(_ac_WebManag.get());
    _TrayMenu->addAction(_ac_Support.get());
    _TrayMenu->addAction(_ac_Bug.get());
    _TrayMenu->addAction(_ac_Earn.get());
    _TrayMenu->addSeparator();

    //_TrayMenu->addAction(_ac_About.get());
    _TrayMenu->addAction(_ac_Logout.get());
    _TrayMenu->addAction(_ac_Close.get());
}

void SjMainWindow::CreateActions()
{
    _ac_Connect.reset(new QAction(//QIcon(":/icons-tm/connect-red.png"),
                          tr("&Connect"), this));
    connect(_ac_Connect.get(), SIGNAL(triggered()), this, SLOT(ac_Connect()));

    _ac_ConnectTo.reset(new QAction(//QIcon(":/icons-tm/connect-grey.png"),
                            tr("&Connect to ..."), this));
    _ac_ConnectTo->setEnabled(false);
    //connect(_ac_ConnectTo.get(), SIGNAL(triggered()), this, SLOT(ac_ConnectTo()));

    _ac_Disconnect.reset(new QAction(//QIcon(":/icons-tm/disconnect-grey.png"),
                             tr("&Disconnect"), this));
    connect(_ac_Disconnect.get(), SIGNAL(triggered()), this, SLOT(ac_Disconnect()));

    _ac_Status.reset(new QAction(//QIcon(":/icons-tm/status-red.png"),
                         tr("&Status"), this));
    connect(_ac_Status.get(), SIGNAL(triggered()), this, SLOT(ac_Status()));

    _ac_Jump.reset(new QAction(//QIcon(":/icons-tm/status-grey.png"),
                       tr("&Jump to Faster"), this));
    connect(_ac_Jump.get(), SIGNAL(triggered()), this, SLOT(ac_Jump()));

    _ac_SwitchCountry.reset(new QAction(//QIcon(":/icons-tm/country-grey.png"),
                                tr("Switch Country"), this));
    connect(_ac_SwitchCountry.get(), SIGNAL(triggered()), this, SLOT(ac_SwitchCountry()));

    _ac_Settings.reset(new QAction(//QIcon(":/icons-tm/settings-red.png"),
                           tr("Se&ttings"), this));
    connect(_ac_Settings.get(), SIGNAL(triggered()), this, SLOT(ac_Settings()));

    _ac_Logs.reset(new QAction(//QIcon(":/icons-tm/logs-red.png"),
                       tr("&Logs"), this));
    connect(_ac_Logs.get(), SIGNAL(triggered()), this, SLOT(ac_Logs()));

    _ac_WebManag.reset(new QAction(//QIcon(":/icons-tm/webmanag-red.png"),
                           tr("&Web Management"), this));
    connect(_ac_WebManag.get(), SIGNAL(triggered()), this, SLOT(ac_WebManag()));

    _ac_Support.reset(new QAction(//QIcon(":/icons-tm/support-red.png"),
                          tr("&Feedback/Support"), this));
    connect(_ac_Support.get(), SIGNAL(triggered()), this, SLOT(ac_Support()));

    _ac_Bug.reset(new QAction(//QIcon(":/icons-tm/bug-red.png"),
                      tr("&Report Bug"), this));
    connect(_ac_Bug.get(), SIGNAL(triggered()), this, SLOT(ac_Bug()));

    _ac_Earn.reset(new QAction(//QIcon(":/icons-tm/earn-red.png"),
                       tr("&Earn Money"), this));
    connect(_ac_Earn.get(), SIGNAL(triggered()), this, SLOT(ac_Earn()));

    //_ac_About.reset(new QAction(tr("&About"), this));
    //connect(_ac_About.get(), SIGNAL(triggered()), this, SLOT(ac_About()));

    _ac_Logout.reset(new QAction(//QIcon(":/icons-tm/close-grey.png"),
                         tr("Logout"), this));
    connect(_ac_Logout.get(), SIGNAL(triggered()), this, SLOT(ac_Logout()));

    _ac_Close.reset(new QAction(//QIcon(":/icons-tm/close-red.png"),
                        tr("Close"), this));
    connect(_ac_Close.get(), SIGNAL(triggered()), this, SLOT(ac_Close()));
}

void SjMainWindow::Icon_Activated(QSystemTrayIcon::ActivationReason )
{
    ++_activatedcount;
    if (_activatedcount == 1 && !_fixed) {
        connect(g_pTheApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(AppFocusChanged(QWidget*, QWidget*)));
        if (NULL == _timer_icon.get()) {
            _timer_icon.reset(new QTimer(this));
            connect(_timer_icon.get(), SIGNAL(timeout()), this, SLOT(updateStateIcon()));
            _timer_icon->start(210);
        }
        updateStateIcon();
    } else {
        DisconnectIconWatcher();
        updateStateIcon();
    }
}

void SjMainWindow::DisconnectIconWatcher()
{
    _fixed = true;
    disconnect(_TrayIcon.get(), SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(Icon_Activated(QSystemTrayIcon::ActivationReason)));
    disconnect(g_pTheApp, SIGNAL(focusChanged(QWidget*, QWidget*)), this, SLOT(AppFocusChanged(QWidget*, QWidget*)));
    if (NULL != _timer_icon.get())
        _timer_icon->stop();
}

void SjMainWindow::AppFocusChanged(QWidget*, QWidget*)
{
    DisconnectIconWatcher();
    updateStateIcon();
}

void SjMainWindow::updateStateIcon()
{
    OpenvpnManager::OvState st = OpenvpnManager::ovsDisconnected;
    if (OpenvpnManager::exists())
        st = OpenvpnManager::Instance()->state();
    updateStateIcon(st);
}

void SjMainWindow::updateStateIcon(OpenvpnManager::OvState st)
{
    QString ic;
    switch (st) {
    case OpenvpnManager::ovsDisconnected:
        ic = OsSpecific::Instance()->IconDisconnected();
        break;
    case OpenvpnManager::ovsConnecting:
        ic = OsSpecific::Instance()->IconConnecting();
        break;
    case OpenvpnManager::ovsConnected:
        ic = OsSpecific::Instance()->IconConnected();
        break;
    default:
        break;
    }
    QIcon icon(ic);
    _TrayIcon->setIcon(icon);
}

void SjMainWindow::FixIcon()
{
    if (!_fixed) {
        _fixed = true;
        DisconnectIconWatcher();
        updateStateIcon();
    }
}

void SjMainWindow::ac_Connect()
{
    FixIcon();
    if (!AuthManager::Instance()->IsLoggedin()) {
        _ConnectAfterLogin = true;
        DoLogin();
    } else {
        DoConnect();
    }
}

void SjMainWindow::ac_ConnectTo()
{
    ac_Connect();	// TODO: -0
    return;

    FixIcon();

    // TODO: -0 pick location

    DoLogin();

//	if (AuthManager::Instance()->IsLoggedin())
//		WndManager::Instance()->ToMap();
}

void SjMainWindow::ac_Disconnect()
{
    FixIcon();
    OpenvpnManager::Instance()->stop();
}

void SjMainWindow::ac_Status()
{
    FixIcon();
    WndManager::Instance()->ToPrimary();
}

void SjMainWindow::ac_Jump()
{
    FixIcon();
    WndManager::Instance()->ToPrimary();
    AuthManager::Instance()->Jump();
}

void SjMainWindow::ac_SwitchCountry()
{
    FixIcon();
    WndManager::Instance()->ToMap();
}

void SjMainWindow::ac_Settings()
{
    FixIcon();
    WndManager::Instance()->ToSettings();
}

void SjMainWindow::ac_Logs()
{
    FixIcon();
    WndManager::Instance()->ToLogs();
}

void SjMainWindow::ac_WebManag()
{
    FixIcon();
    WndManager::Instance()->CloseAll();
    OpenUrl_Panel();
}

void SjMainWindow::ac_Support()
{
    FixIcon();
    WndManager::Instance()->CloseAll();
    OpenUrl_Support();
}

void SjMainWindow::ac_Bug()
{
    FixIcon();
    WndManager::Instance()->CloseAll();
    OpenUrl_Bug();
}

void SjMainWindow::ac_Earn()
{
    FixIcon();
    WndManager::Instance()->CloseAll();
    OpenUrl_Earn();
}

void SjMainWindow::ac_About()
{
    FixIcon();
    WndManager::Instance()->ToPrimary();
}

void SjMainWindow::ac_Close()
{
    FixIcon();
    DoClose();
}

void SjMainWindow::ac_Logout()
{
    FixIcon();
    if (OpenvpnManager::exists())
        OpenvpnManager::Instance()->stop();
    if (AuthManager::IsExists())
        AuthManager::Instance()->DoLogout();
    WndManager::Instance()->ToPrimary();
    ClearConnecttoMenu();
    _ct_menu->setEnabled(false);
    //_ct_menu->setIcon(QIcon(":/icons-tm/connect-grey.png"));
    DisableButtonsOnLogout();
}

std::auto_ptr<SjMainWindow> SjMainWindow::_inst;
SjMainWindow * SjMainWindow::Instance()
{
    if (!_inst.get())
        _inst.reset(new SjMainWindow());
    return _inst.get();
}

void SjMainWindow::DoClose()
{

// HACK: workaround: without any form visible program exits => force show() primary
//	if (WndManager::Instance()->ScrVisible() != NULL)
    WndManager::Instance()->ToPrimary();
    int res = WndManager::Instance()->Confirmation("Would you like to shut Safejumper down?");
    if (res == QDialog::Accepted) {
        WndManager::Instance()->CloseAll();
        if (OpenvpnManager::exists())
            OpenvpnManager::Instance()->stop();
        g_pTheApp->quit();
    }
}

void SjMainWindow::Cleanup()
{
    std::auto_ptr<SjMainWindow> d(_inst.release());
}

void SjMainWindow::ToScr_Primary()
{
    WndManager::Instance()->ToPrimary();
}

void SjMainWindow::on_cancelButton_clicked()
{
    DoCancelLogin();
}

void SjMainWindow::on_loginButton_clicked()
{
    DoLogin();
}

void SjMainWindow::DoCancelLogin()
{
    _CancelLogin = true;
    AuthManager::Instance()->CancelLogin();
}

void SjMainWindow::DoLogin()
{
    if (!AuthManager::Instance()->IsLoggedin()) {
        if (!ui->eLogin->text().isEmpty()) {
            _CancelLogin = false;
            enableButtons(false);
            AuthManager::Instance()->DoLogin(ui->eLogin->text(), ui->ePsw->text());
        }
    }
}

void SjMainWindow::enableButtons(bool enabled)
{
    ui->loginButton->setVisible(enabled);
    ui->cancelButton->setVisible(!enabled);
    ui->eLogin->setEnabled(enabled);
    ui->ePsw->setEnabled(enabled);
    ui->optionsButton->setEnabled(enabled);
}

void SjMainWindow::DoConnect()
{
    OpenvpnManager::Instance()->start();
}

void SjMainWindow::Finished_ObfsName()
{
    QString errmsg;
    bool ok = AuthManager::Instance()->ProcessXml_ObfsName(errmsg);
    if (!ok)
        log::logt(errmsg);
}

void SjMainWindow::Finished_EccName()
{
    QString errmsg;
    bool ok = AuthManager::Instance()->ProcessXml_EccName(errmsg);
    if (!ok)
        log::logt(errmsg);
    else {
        int enc = Setting::Encryption();
        if (ENCRYPTION_RSA != enc)
            Setting::Instance()->LoadServer();
    }
}

//void SjMainWindow::Finished_EccxorName()
//{
//	QString errmsg;
//	bool ok = AuthManager::Instance()->ProcessXml_EccxorName(errmsg);
//	if (!ok)
//		log::logt(errmsg);
//}

void SjMainWindow::AccTypeFinished()
{
    QString errmsg;
    bool ok = AuthManager::Instance()->ProcessAccountXml(errmsg);
    if (!ok)
        log::logt(errmsg);
}

void SjMainWindow::ExpireFinished()
{
    QString errmsg;
    bool ok = AuthManager::Instance()->ProcessExpireXml(errmsg);
    if (!ok)
        log::logt(errmsg);
}

void SjMainWindow::LoginFinished()
{
    QString msg;

    bool ok = AuthManager::Instance()->ProcessXml_Servers(msg);
    if (ok) {
        SaveCreds();

        if (Setting::Encryption() == ENCRYPTION_RSA)
            Setting::Instance()->LoadServer();

        // TODO: -0
//		{
//			SETTINGS_OBJ;
//			int ix = settings.value("dd_Protocol_ix", -1).toInt();
//			if (ix < 0)
//			{
//log::logt("sleeping 3");
//				QThread::sleep(3);		// if first run: wait for pings  - to get adequate jump
//			}
//		}
        WndManager::Instance()->ToPrimary();
        EnableButtonsOnLogin();
        ConstructConnecttoMenu();
        if (_ConnectAfterLogin)
            DoConnect();
    } else {
        DisableButtonsOnLogout();
        if (!_CancelLogin) {
            WndManager::Instance()->ToFront(this);
            log::logt("Login Error " + msg);
            Dlg_Error dlg(msg, "Login Error", this);
            dlg.exec();
        }
    }
    enableButtons(true);
    _ConnectAfterLogin = false;
}

void SjMainWindow::CreateMenuItem(QMenu * m, const QString & name, size_t srv)
{
    QAction * a = m->addAction(name);
    AcConnectto * o = new AcConnectto(srv);
    _connectto.push_back(o);
    this->connect(a, SIGNAL(triggered()), o, SLOT(ac_ConnectTo()));
}

void SjMainWindow::ConstructConnecttoMenu()
{
    if (AuthManager::IsExists()) {
        AuthManager * am = AuthManager::Instance();
        if (am->IsLoggedin()) {
            ClearConnecttoMenu();

            const std::vector<size_t> & hubs = am->GetHubs();
            if (_ct_menu.get() == NULL) {	// one time during entire program run
                _ct_menu.reset(_TrayMenu->addMenu("Connect to ..."));
                _TrayMenu->removeAction(_ac_ConnectTo.get());
                _TrayMenu->insertMenu(_ac_Disconnect.get(), _ct_menu.get());
            }
            _ct_menu->setEnabled(true);
            //_ct_menu->setIcon(QIcon(":/icons-tm/connect-red.png"));

            if (!Setting::Instance()->IsShowNodes()) {
                for (size_t k = 0; k < hubs.size(); ++k) {
                    AServer sr = am->GetSrv(hubs[k]);
                    CreateMenuItem(_ct_menu.get(), sr.name, hubs[k]);//am->ServerIdFromHubId(k));
                }
            } else {
                const std::vector<std::pair<bool, int> > & L0 = am->GetLvl0();
                for (size_t k = 0; k < L0.size(); ++k) {
                    if (L0[k].first) {
                        // hub - add submenu
                        int idhub = L0[k].second;
                        AServer h = am->GetHub(idhub);
                        QMenu * m = new QMenu(h.name);
                        _hub_menus.push_back(m);

                        // add into it all individual servers
                        const std::vector<int> & L1 = am->GetLvl1(idhub);
                        for (size_t k = 0; k < L1.size(); ++k) {
                            int idsrv = L1[k];
                            if (idsrv > -1) {
                                AServer se = am->GetSrv(idsrv);
                                CreateMenuItem(m, se.name, idsrv);
                            }
                        }
                        _ct_menu->addMenu(m);
                    } else {	// just a server without hub
                        int idsrv = L0[k].second;
                        AServer se = am->GetSrv(idsrv);
                        CreateMenuItem(_ct_menu.get(), se.name, idsrv);
                    }
                }

            }
        }
    }
}

void SjMainWindow::ClearConnecttoMenu()
{
    for (size_t k = 0; k < _hub_menus.size(); ++k)
        delete _hub_menus.at(k);
    _hub_menus.clear();

    // remove handlers
    if (!_connectto.empty()) {
        for (size_t k = 0; k < _connectto.size(); ++k)
            delete _connectto[k];
        _connectto.clear();
    }

    // destroy menu items
    if (_ct_menu.get())
        if (!_ct_menu->isEmpty())
            _ct_menu->clear();			// delete actions
}

void SjMainWindow::on_optionsButton_clicked()
{
    WndManager::Instance()->ToSettings();
}

void SjMainWindow::StatusConnecting()
{
    DisableMenuItems(true);
    updateStateIcon(OpenvpnManager::ovsConnecting);
}

void SjMainWindow::StatusConnected()
{
    updateStateIcon(OpenvpnManager::ovsConnected);
    _ac_Jump->setEnabled(true);			//_ac_Jump->setIcon(QIcon(":/icons-tm/jump-red.png"));
    _ac_SwitchCountry->setEnabled(true);	//_ac_SwitchCountry->setIcon(QIcon(":/icons-tm/country-red.png"));
}

void SjMainWindow::StatusDisconnected()
{
    DisableMenuItems(false);
    updateStateIcon(OpenvpnManager::ovsDisconnected);
}

static void s_set_enabled(QAction * ac, bool enabled, const char * icon_word)
{
    if (!ac)
        return;
    ac->setEnabled(enabled);
//	QString fn = ":/icons-tm/";
//	fn += icon_word;
//	fn += (enabled ? "-red.png" : "-grey.png");
//	ac->setIcon(QIcon(fn));
}
void SjMainWindow::DisableMenuItems(bool connecting)
{
    QAction * ar[] = {_ac_Connect.get(), _ac_Jump.get()};
    static const char * words[] = {"connect", "jump"};
    for (size_t k = 0; k < sizeof(words) / sizeof(words[0]); ++k)
        s_set_enabled(ar[k], !connecting, words[k]);

    static const char * country = "country";
    static const char * disconn = "disconnect";
    static const char * conn = "connect";
    if (AuthManager::Instance()->IsLoggedin()) {
        s_set_enabled(_ac_SwitchCountry.get(), !connecting, country);
        s_set_enabled(_ac_Disconnect.get(), connecting, disconn);
        s_set_enabled(_ac_ConnectTo.get(), !connecting, conn);

    } else {
        s_set_enabled(_ac_SwitchCountry.get(), false, country);
        s_set_enabled(_ac_Disconnect.get(), false, disconn);
        s_set_enabled(_ac_ConnectTo.get(), false, conn);
    }
}

void SjMainWindow::Finished_Updates()
{
    AuthManager::Instance()->ProcessUpdatesXml();
}

void SjMainWindow::Finished_OldIpHttp()
{
    AuthManager::Instance()->ProcessOldIpHttp();
}

void SjMainWindow::Finished_OldIp(const QString & s)
{
    log::logt("Finished_OldIp: " + s);
    AuthManager::Instance()->ProcessOldIp(s);
}

void SjMainWindow::Finished_Dns()
{
    AuthManager::Instance()->ProcessDnsXml();
}

void SjMainWindow::StartWifiWatcher()
{
    if (NULL == _timer_wifi.get()) {
        _wifi_processing = false;
        _timer_wifi.reset(new QTimer());
        connect(_timer_wifi.get(), SIGNAL(timeout()), this, SLOT(Timer_WifiWatcher()));
        _timer_wifi->start(5000);
    }
}

void SjMainWindow::StopWifiWatcher()
{
    if (NULL != _timer_wifi.get()) {
        QTimer * t = _timer_wifi.release();
        t->stop();
        t->deleteLater();
    }
}

void SjMainWindow::Timer_WifiWatcher()
{
    if (NULL != _timer_wifi.get() 		// if not terminating now
            && !_wifi_processing) {			// and not already in the body below
        _wifi_processing = true;
        bool stopped = false;
        if (!AuthManager::IsExists()) {
            stopped = true;
        } else {
            if (!OpenvpnManager::exists()) {
                stopped = true;
            } else {
                if (OpenvpnManager::Instance()->state() == OpenvpnManager::ovsDisconnected)
                    stopped = true;
            }
        }

        if (stopped) {
            if (!AuthManager::Instance()->IsLoggedin()) {
                if (Setting::Instance()->IsAutoconnect()) {	// log in only if checked Auto-connect when app starts
                    if (OsSpecific::Instance()->HasInsecureWifi()) {
                        _ConnectAfterLogin = true;
                        DoLogin();
                    }
                }
            } else {
                if (OsSpecific::Instance()->HasInsecureWifi())
                    DoConnect();
            }
        }
        _wifi_processing = false;
    }
}

void SjMainWindow::BlockOnDisconnect()
{
    // implementation is the same as in the old Safejumper
    bool doblock = false;
    if (Setting::Instance()->IsBlockOnDisconnect()) {
        if (AuthManager::IsExists()) {
            if (!AuthManager::Instance()->IsLoggedin()) {
                doblock = true;
            } else {
                if (!OpenvpnManager::exists()) {
                    doblock = true;
                } else {
                    if (OpenvpnManager::Instance()->state() == OpenvpnManager::ovsDisconnected)
                        doblock = true;
                    // otherwise unblocked and should be unblocked
                }
            }
        }
    }

    if (doblock) {
        OsSpecific::Instance()->NetDown();
    }
}
