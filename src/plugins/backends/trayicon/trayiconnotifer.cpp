#include "trayiconnotifer.h"
#include "core/snore.h"

#include <QtCore>
#include <QSystemTrayIcon>
#include <QTimer>
#include <QTime>
#include <QDebug>
using namespace Snore;

Q_EXPORT_PLUGIN2(trayicon,TrayIconNotifer)

TrayIconNotifer::TrayIconNotifer () :
    SnoreBackend ( "SystemTray" ),
    m_trayIcon(NULL),
    m_id ( 0 ),
    m_displayed(-1)
{

}

bool TrayIconNotifer::init(SnoreCore *snore){    
    m_trayIcon = snore->trayIcon();
    if(m_trayIcon == NULL)
        return false;
    connect(m_trayIcon,SIGNAL(messageClicked()),this,SLOT(actionInvoked()));
    return SnoreBackend::init(snore);
}

void TrayIconNotifer::slotRegisterApplication ( Application *application )
{
    Q_UNUSED ( application )
}
void TrayIconNotifer::slotUnregisterApplication ( Application *application )
{
    Q_UNUSED ( application )
}

uint TrayIconNotifer::slotNotify ( Notification notification )
{
    m_notificationQue.append(notification);
    if(m_lastNotify.elapsed()> Notification::DefaultTimeout * 1000){
        displayNotification();
    }
    return m_id++;
}

bool TrayIconNotifer::slotCloseNotification( Notification notification )
{
    Q_UNUSED ( notification )
    return false;
}

void TrayIconNotifer::displayNotification(){
    qDebug()<<"Display"<<m_notificationQue.size();
    Notification notification =  m_notificationQue.takeFirst();
    if(!m_notificationQue.isEmpty()){
        QTimer::singleShot(notification.timeout()*1000,this,SLOT(slotCloseNotification()));
    }

    qDebug()<<"taking"<<notification.title();
    m_displayed = notification.id();
    m_trayIcon->showMessage ( Notification::toPlainText(notification.title()),Notification::toPlainText(notification.text()),QSystemTrayIcon::NoIcon,notification.timeout() *1000 );
    m_lastNotify.restart();
}

void TrayIconNotifer::slotCloseNotification(){
    Notification n = snore()->getActiveNotificationByID(m_displayed);
    if(n.isValid()){
        closeNotification(n,NotificationEnums::CloseReasons::TIMED_OUT);
    }
    displayNotification();
}

void TrayIconNotifer::actionInvoked(){
    qDebug()<<"Traicon invoked"<<m_displayed;

    Notification n = snore()->getActiveNotificationByID(m_displayed);
    if(n.isValid()){
        if(n.actions().isEmpty()){
            n.setActionInvoked(n.actions().keys().first());
            snore()->notificationActionInvoked(n);
        }
        closeNotification(n,NotificationEnums::CloseReasons::CLOSED);

    }

}


#include "trayiconnotifer.moc"