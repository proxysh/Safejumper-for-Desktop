#ifndef PINGWAITER_H
#define PINGWAITER_H

#include <QObject>
#include <QProcess>

class PingWaiter : public QObject
{
	Q_OBJECT
public:
	explicit PingWaiter(size_t idWaiter, QObject *parent = 0);
	~PingWaiter();

private:
	size_t _idWaiter;

signals:

public slots:
	void PingFinished(int exitCode, QProcess::ExitStatus exitStatus);
	void PingError(QProcess::ProcessError);
	void Timer_Terminate();
};

#endif // PINGWAITER_H
