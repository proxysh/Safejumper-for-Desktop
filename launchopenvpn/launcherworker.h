#ifndef LAUNCHERWORKER_H
#define LAUNCHERWORKER_H

#include <QObject>

class LauncherWorker : public QObject
{
	Q_OBJECT
public:
	explicit LauncherWorker(const QString & pfnParams, QObject *parent = 0);
	~LauncherWorker();

signals:

public slots:
	void Timer_Constructed();

private:
	QString _pfnParams;
};

#endif // LAUNCHERWORKER_H
