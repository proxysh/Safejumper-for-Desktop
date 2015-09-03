#ifndef ACCONNECTTO_H
#define ACCONNECTTO_H

#include <QObject>

// handles dynamic menu mouse click and redirects to controller with id of hub/srv
class AcConnectto : public QObject
{
	Q_OBJECT
public:
	explicit AcConnectto(size_t srvid, QObject *parent = 0);
	~AcConnectto();

private:
	size_t _srvid;
signals:

public slots:
	void ac_ConnectTo();
};

#endif // ACCONNECTTO_H
