#include "cstdio"
#include "cstdlib"
#include "cstring"

#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QFuture>
#include <QSettings>

#include <QDebug>

#include "cntlmwrapper.h"

#include "libcntlm/libcntlm.h"

CntlmWrapper::CntlmWrapper(QObject *parent) : QObject(parent)
{

    QSettings settings;
    listen = settings.value("listen", "8080").toString();
    user = settings.value("user", "user@domain.com").toString();
    password = settings.value("password", "SimplePassword").toString();
    proxy = settings.value("proxy", "10.0.0.1:8080").toString();

    running = false;
}

QString CntlmWrapper::getUser() const
{
    return user;
}

void CntlmWrapper::setUser(const QString &value)
{
    user = value;
}

QString CntlmWrapper::getPassword() const
{
    return password;
}

void CntlmWrapper::setPassword(const QString &value)
{
    password = value;
}

void CntlmWrapper::start()
{
    if (memorizeSettings) {
        QSettings settings;
        settings.setValue("listen", listen);
        settings.setValue("user", user);
        settings.setValue("password", password);
        settings.setValue("proxy", proxy);
        settings.sync();
    }

    running = true;
    Q_EMIT(runningChanged(running));

    QFuture<void> f = QtConcurrent::run(cntlm_start, listen, user, password, proxy);

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>();
    watcher->setFuture(f);
    connect(watcher, &QFutureWatcher<void>::finished, [=]() {
        qDebug() << "FINISHED";
        running = false;
        Q_EMIT(runningChanged(running));

        watcher->deleteLater();
    });
}

void CntlmWrapper::stop()
{
    libcntlm::stop();
}

void CntlmWrapper::cntlm_start(QString listen, QString user, QString password, QString proxy)
{
    char * argsStr = (char*) malloc(sizeof(char) * 1024);
    sprintf(argsStr, "cntlm -vfl %s -u %s -p %s %s", listen.toLatin1().data(), user.toLatin1().data(), password.toLatin1().data(), proxy.toLatin1().data());

    qDebug() << argsStr;
    char ** cntlm_args = NULL;
    int argc = libcntlm::strsplit(argsStr, ' ', &cntlm_args);
    qDebug() << "Starting with "<< argc << "args ";
    /*for(int i = 0; i < argc; i ++)
        qDebug() << cntlm_args[i];*/

    int result = libcntlm::start(argc, cntlm_args);
    free(argsStr);
    qDebug() << "ended" << result;
}

bool CntlmWrapper::getMemorizeSettings() const
{
    return memorizeSettings;
}

void CntlmWrapper::setMemorizeSettings(bool value)
{
    memorizeSettings = value;
}

QString CntlmWrapper::getProxy() const
{
    return proxy;
}

void CntlmWrapper::setProxy(const QString &value)
{
    proxy = value;
}

QString CntlmWrapper::getListen() const
{
    return listen;
}

void CntlmWrapper::setListen(const QString &value)
{
    listen = value;
}

bool CntlmWrapper::getRunning() const
{
    return running;
}