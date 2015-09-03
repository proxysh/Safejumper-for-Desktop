#ifndef PORTFORWARDER_H
#define PORTFORWARDER_H

#include <memory>

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "common.h"

// will be queued in the main loop and just chain rquests for port forwarding
class PortForwarder : public QObject
{
	Q_OBJECT
public:
	explicit PortForwarder(const UVec & ports, QNetworkAccessManager & nam, const QString & login, const QString & psw, QObject *parent = 0);
	~PortForwarder();

	void StartFirst();
signals:

public slots:
	void FinishedFirstPage();
	void Finished_SetPort();
private:
	UVec _ports;
	const QString & _login;
	const QString & _psw;
	QNetworkAccessManager & _nam;

	std::auto_ptr<QNetworkReply> _reply;
	IIMap _to_set;	// <port slot IX in the API (1-based), port>
	IIMap::iterator _it_curr;

	void SetPort();
};

#endif // PORTFORWARDER_H
