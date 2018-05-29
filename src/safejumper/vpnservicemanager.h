/***************************************************************************
 *   Copyright (C) 2017 by Jeremy Whiting <jeremypwhiting@gmail.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation version 2 of the License.                *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef VPNSERVICEMANAGER_H
#define VPNSERVICEMANAGER_H

#include "common.h"

#include <QLocalSocket>
#include <QObject>
#include <QTimer>

/*!
 * @brief A class acting as a mediator between the client (GUI) and the actual backend
 * This class wraps the requests into a custom command to be sent to the service
 */
class VPNServiceManager : public QObject
{
    Q_OBJECT
public:
    static VPNServiceManager * instance();
    static bool exists();
    static void cleanup();

    virtual ~VPNServiceManager();

    vpnState state();

    void startPortLoop(bool changePort);
signals:
    /*!
     * Notify the client who's been registered on the status change
     * of the connection initiated by @ref sendConnectToVPNRequest, @ref sendStatusRequest or @ref sendDisconnectFromVPNRequest
     * @param status The status that the connection entered
     */
    void stateChanged(vpnState state);

    /*!
     * Notify the client of status word changes
     * @param word The status word
     */
    void stateWord(OpenVPNStateWord word);

    /*!
     * A signal emitted in case of an error
     * @param description The description of the error
     */
    void error(QString description);

    /*!
     * \brief serviceUnavailable
     * Signal emitted when we are unable to connect to vpn service socket
     */
    void serviceUnavailable();

    /*!
     * \brief nextPort
     * Signal that openvpn timed out so switch to next port or node
     */
    void timedOut();

    void gotNewIp(const QString &ip);

    void killSwitch();

public slots:
    /*!
     * Query the status of the connection
     */
    void sendStatusRequest();

    /*!
     * \brief sendCredentials
     * Tells service our vpn credentials
     */
    void sendCredentials();

    /*!
     * Connect to the VPN service with the settings given
     */
    void sendConnectToVPNRequest();

    /*!
     * Send a disconnect command to the service
     */
    void sendDisconnectFromVPNRequest();

    /*!
     * \brief killRunningOpenvpn
     * Ask service to kill any running openvpn processes if there are any
     */
    void killRunningOpenvpn();

    void sendNetdownCommand();

private slots:
    void socket_readyRead();
    void socket_disconnected();
    void socket_error(QLocalSocket::LocalSocketError error);

private:
    explicit VPNServiceManager(QObject *parent = 0);

    /*!
     * \brief sendCommand
     * \param object
     * Send given object to serice
     */
    void sendCommand(const QJsonObject &object);

    bool ensureConnected();

    /*!
     * Connect to the core service
     */
    bool connectToCore();

    /*!
     * Disconnects from the core service
     */
    void disconnectFromCore();

    void tryNextPort();

    QLocalSocket m_socket;

    bool mConnected;

    static VPNServiceManager *mInstance;

    vpnState mState;
    bool mChangingPorts;
    bool mInPortLoop;
    bool mPortDialogShown;
    bool mUserRequestedDisconnect;
};

#endif // QBCVPNSERVICECONNECTION_H
