#include "portforwarder.h"

#include <QFile>
#include <QDomNodeList>

#include "log.h"
#include "common.h"

PortForwarder::PortForwarder(const UVec & ports, QNetworkAccessManager & nam, const QString & login, const QString & psw, QObject *parent)
	: QObject(parent)
	, _ports(ports)
	, _nam(nam)
	, _login(login)
	, _psw(psw)
{}

PortForwarder::~PortForwarder()
{
	if (NULL != _reply.get())
	{
		if (_reply->isRunning())
		{
			this->disconnect(_reply.get());
			this->disconnect(this);
			_reply->abort();			// emit finished()
		}
	}
}

/*

	https://api.proxy.sh/safejumper/get_ports/spaxugci/04w8z8aWjS
	- gets ports, set for forwarding

	https://api.proxy.sh/safejumper/ports/port1:2234/spaxugci/04w8z8aWjS
	- sets port1 for forwarding

*/

void PortForwarder::StartFirst()
{
	_reply.reset(_nam.get(BuildRequest(QUrl("https://api.proxy.sh/safejumper/get_ports/" + QUrl::toPercentEncoding(_login, "", "") + "/" + QUrl::toPercentEncoding(_psw, "", "")))));
	this->connect(_reply.get(), SIGNAL(finished()), this, SLOT(FinishedFirstPage()));
}

void PortForwarder::FinishedFirstPage()
{
	bool err = false;
	if (_reply->error() != QNetworkReply::NoError)
	{
		err = true;
		log::logt("FinishedFirstPage(): got QNetworkReply::Error");
	}
	else
	{
		QByteArray ba = _reply->readAll();
		if (ba.isEmpty())
		{
			err = true;
			log::logt("FinishedFirstPage(): Server's answer is empty");
		}

		QString msg;
		QDomDocument doc;
		if (!doc.setContent(QString(ba), &msg))
		{
			err = true;
			msg = "Error parsing server XML response\n" + msg;
			log::logt(msg);
		}

		if (!err)
		{
/*
QFile f("/tmp/ppp.xml");
f.open(QIODevice::WriteOnly);
f.write(ba);
f.flush();
f.close();

<?xml version="1.0"?>
<root>
	<port1>0</port1>
	<port2>0</port2>
	<port3>0</port3>
	<port4>0</port4>
	<port5>0</port5>
</root>
*/

			UVec already;
			USet already_set;
			bool loop = true;
			for (int k = 1; loop && !err && k <= PORT_FORWARD_MAX; ++k)
			{
				QDomNodeList nl = doc.elementsByTagName("port" + QString::number(k));
				if (nl.size() > 0)
				{
					QDomNode n = nl.item(0);
					QString ts = n.toElement().text();
					bool ok;
					int t = ts.toInt(&ok);
					if (!ok)
						err = true;
					if (t > 0)
					{
						already.push_back(t);
						already_set.insert(t);
					}
					else
					{
						if (err)
							log::logt("Cannot parse port" + QString::number(k) + " '" + ts + "'");
					}
				}
				else
				{
					loop = false;
				}
			}

			if (!err)
			{
//typedef std::array<uint16_t, PORT_FORWARD_MAX> IAr;
				UVec aalready;
				UVec atoset;
				aalready.assign(PORT_FORWARD_MAX, 0);
				atoset.assign(PORT_FORWARD_MAX, 0);
//for (int k =0; k < PORT_FORWARD_MAX; ++k) aalready[k] = atoset[0] = 0;
				for (size_t k =0; k < already.size() && k < PORT_FORWARD_MAX; ++k)
					aalready[k] = already[k];
				std::sort(_ports.begin(), _ports.end());
				for (size_t k =0; k < _ports.size() && k < PORT_FORWARD_MAX; ++k)
					atoset[k] = _ports[k];

				IIMap to_set;
				for (size_t k = 0; k < PORT_FORWARD_MAX; ++k)
				{
					if (aalready[k] != atoset[k])
					{
						to_set.insert(std::make_pair(k+1, atoset[k]));			// take into account 0 - remove port forwarding
						break;
					}
				}

				std::swap(to_set, _to_set);
				_it_curr = _to_set.begin();
				if (!_to_set.empty() && !err)
					SetPort();
			}
#if 0
			bool has_new = false;
			for (size_t k = 0; k < _ports.size(); ++k)
			{
				if (already_set.find(_ports.at(k)) == already_set.end())
				{
					has_new = true;
					break;
				}
			}

			if (has_new && !err)
			{
				UVec already_sorted(already);
				std::sort(already_sorted.begin(), already_sorted.end());
				std::sort(_ports.begin(), _ports.end());
				IIMap to_set;		// <port slot IX in the API (1-based), port>
				for (size_t k = 0; k < _ports.size() && k < PORT_FORWARD_MAX; ++k)
				{
					bool is_already = false;
					if (already.size() > k)
						if (already.at(k) == _ports.at(k))
							is_already = true;
					if (!is_already)
					{
						to_set.insert(std::make_pair(k+1, _ports.at(k)));
					}
				}

				std::swap(to_set, _to_set);
				_it_curr = _to_set.begin();
				SetPort();
			}
#endif
		}
	}
}

void PortForwarder::SetPort()
{
	if (_it_curr != _to_set.end())
	{
		int ix = (*_it_curr).first;
		int port = (*_it_curr).second;
		if (_reply.get())
		{
			if (_reply->isRunning())
			{
				log::logt("SetPort(): Connection is still open when we want to set next port" + QString::number(ix) + " " + QString::number(port));
				_reply->abort();			// emit finished()
			}
		}

		log::logt("Forwarding port" + QString::number(ix) + ": " + QString::number(port));
		// https://api.proxy.sh/safejumper/ports/port1:2234/spaxugci/04w8z8aWjS
		// - sets port1 for forwarding
		_reply.reset(_nam.get(BuildRequest(QUrl("https://api.proxy.sh/safejumper/ports/port"
			+ QString::number(ix) + ":" + QString::number(port) + "/"
			+ QUrl::toPercentEncoding(_login, "", "") + "/" + QUrl::toPercentEncoding(_psw, "", "")))));
		this->connect(_reply.get(), SIGNAL(finished()), this, SLOT(Finished_SetPort()));
	}
}

void PortForwarder::Finished_SetPort()
{
	bool err = false;
	if (_reply->error() != QNetworkReply::NoError)
	{
		err = true;
		log::logt("Finished_SetPort(): got QNetworkReply::Error");
		// TODO: -2 handle networking error - resubmit port forward request
	}
	else
	{
		QByteArray ba = _reply->readAll();
		if (ba.isEmpty())
		{
			err = true;
			log::logt("FinishedFirstPage(): Server's answer is empty");
		}

		if (!err)
		{

			// <?xml version="1.0"?>
			// <root><ok>200</ok></root>

//QFile f("/tmp/ppp-response.xml");
//f.open(QIODevice::WriteOnly);
//f.write(ba);
//f.flush();
//f.close();
		}

		// ignore empty response - process next port
		_it_curr++;
		SetPort();
	}
}


