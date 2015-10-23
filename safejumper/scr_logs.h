#ifndef SCR_LOGS_H
#define SCR_LOGS_H

#include <QDialog>
#include <QCloseEvent>
#include <memory>

namespace Ui {
class Scr_Logs;
}

class Scr_Logs : public QDialog
{
	Q_OBJECT

public:
	~Scr_Logs();
	static Scr_Logs * Instance();
	static bool IsExists() { return (_inst.get() != NULL); }
	static void Cleanup() { if (_inst.get() != NULL) delete _inst.release();}

	void Log(const QString & s);
public slots:
	void Clicked_Copytoclipboard();
private slots:
	void ShowSupportUrl();
	void ToScr_Connect();
	void Pressed_Head();
	void Clicked_Min();
	void Clicked_Cross();

protected:
	virtual void closeEvent(QCloseEvent * event);
	virtual void keyPressEvent(QKeyEvent * event);
	virtual bool eventFilter(QObject *obj, QEvent *event);
private:
	Ui::Scr_Logs *ui;
	explicit Scr_Logs(QWidget *parent = 0);
	static std::auto_ptr<Scr_Logs> _inst;
	bool _moving;
	QPoint _WndStart;
	QPoint _CursorStart;
};

#endif // SCR_LOGS_H
