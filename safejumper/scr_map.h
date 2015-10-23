#ifndef SCR_MAP_H
#define SCR_MAP_H

#include <memory>

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class Scr_Map;
}

#define PROTOCOL_SELECTION_STR "-- Select protocol & port --"

class Scr_Map : public QDialog
{
	Q_OBJECT

public:
	static Scr_Map * Instance();
	static bool IsExists() { return (_inst.get() != NULL); }
	static void Cleanup() { if (_inst.get() != NULL) delete _inst.release();}
	~Scr_Map();

	int CurrSrv();	  // -1 if not selected, otherwise [0-...] id of server inside auth manager
	int CurrProto();	// -1 if not selected, otherwise [0-...] id of protocol inside Settings

	void RePopulateLocations();		// TODO: -1 methods to update particular row with new ping/load%
	void SetServer(int ixsrv);
	void SetProtocol(int ix);
	void StatusConnecting();
	void StatusConnected();
	void StatusDisconnected();

private:
	Ui::Scr_Map *ui;
	static std::auto_ptr<Scr_Map> _inst;
	explicit Scr_Map(QWidget *parent = 0);
	bool _IsShowNodes;	  // remember for which srv/hub the list was populated; in settings maybe inadequate due to refill during settings change
	void DisplayMark(const QString & name);
	QPoint _default;
	void SetRowStyle(bool show_nodes);
	bool _moving;
	QPoint _WndStart;
	QPoint _CursorStart;

private slots:
	void ToScr_Connect();
	void ToScr_Settings();
	void Changed_dd_Protocol(int ix);
	void Changed_dd_Sever(int ix);
	void Clicked_b_Connect();

	void Clicked_b_Tmp();
	void Changed_xy();
public slots:
	void Pressed_Head();
	void Clicked_Min();
	void Clicked_Cross();
protected:
	virtual void closeEvent(QCloseEvent * event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual void mouseMoveEvent(QMouseEvent *event);
	virtual bool eventFilter(QObject *obj, QEvent *event);
	QPoint _curr;

};

#endif // SCR_MAP_H
