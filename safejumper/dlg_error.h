#ifndef DLG_ERROR_H
#define DLG_ERROR_H

#include <QDialog>

namespace Ui {
class Dlg_Error;
}

class Dlg_Error : public QDialog
{
	Q_OBJECT

public:
	explicit Dlg_Error(const QString & msg, const QString & caption, QWidget *parent = 0);
	~Dlg_Error();
private slots:
	void _ac_Close();
	void _ac_Support();
private:
	Ui::Dlg_Error *ui;
	void DoClose();
protected:
	void resizeEvent(QResizeEvent *event);
};

#endif // DLG_ERROR_H
