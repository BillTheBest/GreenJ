/****************************************************************************
**
** Copyright (C) 2012 Lorem Ipsum Mediengesellschaft m.b.H.
**
** GNU General Public License
** This file may be used under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation and
** appearing in the file LICENSE.GPL included in the packaging of this file.
**
****************************************************************************/

#include <QWebFrame>
#include <QString>
#include "phone/Account.h"
#include "phone/Call.h"
#include "phone/Phone.h"
#include "LogInfo.h"
#include "LogHandler.h"
#include "Config.h"
#include "JavascriptHandler.h"

using phone::Phone;
using phone::Call;
using phone::Account;

//-----------------------------------------------------------------------------
JavascriptHandler::JavascriptHandler(Phone &phone, QWebView *web_view) :
    phone_(phone), web_view_(web_view), js_callback_handler_("")
{
}

//-----------------------------------------------------------------------------
void JavascriptHandler::accountState(const int state) const
{
    evaluateJavaScript("accountStateChanged(" + QString::number(state) + ")");
}

//-----------------------------------------------------------------------------
void JavascriptHandler::callState(const int call_id, const int code, 
                                  const int last_status) const
{
    evaluateJavaScript("callStateChanged(" + QString::number(call_id) + "," 
                                           + QString::number(code) + "," 
                                           + QString::number(last_status) + ")");
}

//-----------------------------------------------------------------------------
void JavascriptHandler::incomingCall(const Call &call) const
{
    evaluateJavaScript("incomingCall(" + QString::number(call.getId()) + ",'" 
                                       + call.getUrl() + "','" 
                                       + call.getName() + "')");
}

//-----------------------------------------------------------------------------
QUrl JavascriptHandler::getPrintPage() const
{
    QVariant url = evaluateJavaScript("getPrintUrl();");

    if (!url.convert(QVariant::Url)) {
        if (!url.isNull()) {
            LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Print page: Wrong url format"));
        }
        return QUrl("about:blank");
    }

    return url.toUrl();
}

//-----------------------------------------------------------------------------
void JavascriptHandler::soundLevel(int level) const
{
    evaluateJavaScript("soundLevel(" + QString::number(level) + ")");
}

//-----------------------------------------------------------------------------
void JavascriptHandler::microphoneLevel(int level) const
{
    evaluateJavaScript("microphoneLevel(" + QString::number(level) + ")");
}

//-----------------------------------------------------------------------------
void JavascriptHandler::logMessage(const LogInfo &info) const
{
    QString json = "{'time':'" + info.time_.toString("dd.MM.yyyy hh:mm:ss")
                 + "','status':" + QString::number(info.status_) 
                 + ",'domain':'" + info.domain_
                 + "','code':" + QString::number(info.code_) 
                 + ",'message':'" + info.msg_ + "'}";

    evaluateJavaScript("logMessage(" + json + ")");
}

//-----------------------------------------------------------------------------
QVariant JavascriptHandler::evaluateJavaScript(const QString &code) const
{
    if (js_callback_handler_.isEmpty()) {
        return web_view_->page()->mainFrame()->evaluateJavaScript(code);
    }
    return web_view_->page()->mainFrame()->evaluateJavaScript(js_callback_handler_ + "." + code);
}

//-----------------------------------------------------------------------------
// Public slots
// These methods will be exposed as JavaScript methods.

//-----------------------------------------------------------------------------
int JavascriptHandler::registerJsCallbackHandler(const QString &handler_name)
{
    js_callback_handler_ = handler_name;
    return 0;
}

//-----------------------------------------------------------------------------
bool JavascriptHandler::checkAccountStatus() const
{
    return phone_.checkAccountStatus();
}

//-----------------------------------------------------------------------------
QVariantMap JavascriptHandler::getAccountInformation() const
{
    return phone_.getAccountInfo();
}

//-----------------------------------------------------------------------------
bool JavascriptHandler::registerToServer(const QString &host, const QString &user_name,
                                         const QString &password) const
{
    LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "register"));

    Account acc;
    acc.setUsername(user_name);
    acc.setPassword(password);
    acc.setHost(host);

    return phone_.registerUser(acc);
}

//-----------------------------------------------------------------------------
void JavascriptHandler::unregisterFromServer() const
{
    LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "unregister"));

    phone_.unregister();
}

//-----------------------------------------------------------------------------
int JavascriptHandler::makeCall(const QString &number) const
{
    Call *call = phone_.makeCall(number);
    if (!call) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "makeCall: failed"));
        return -1;
    }
    LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "calling " + number));
    return call->getId();
}

//-----------------------------------------------------------------------------
void JavascriptHandler::callAccept(const int call_id) const
{
    Call *call = phone_.getCall(call_id);
    if (call) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "accepting call " + QString::number(call_id)));
        call->answerCall();
    } else {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "callAccept: Call doesn't exist!"));
    }
}

//-----------------------------------------------------------------------------
void JavascriptHandler::hangup(const int call_id) const
{    
    Call *call = phone_.getCall(call_id);
    if (call) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "hangup call " + QString::number(call_id)));
        call->hangUp();
    } else {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Hangup: Call doesn't exist!"));
    }
}

//-----------------------------------------------------------------------------
void JavascriptHandler::hangupAll() const
{
    LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "hangup all calls"));
    phone_.hangUpAll();
}

//-----------------------------------------------------------------------------
void JavascriptHandler::setLogLevel(const unsigned int log_level) const
{
    LogHandler::getInstance().setLogLevel(log_level);
}

//-----------------------------------------------------------------------------
QString JavascriptHandler::getCallUserData(const int call_id) const
{
    Call *call = phone_.getCall(call_id);
    if (call) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "Get userdata for call " + QString::number(call_id)));
        return call->getUserData();
    }
    LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "getCallUserData: Call doesn't exist!"));
    return "";
}

//-----------------------------------------------------------------------------
void JavascriptHandler::setCallUserData(const int call_id, const QString &data) const
{
    Call *call = phone_.getCall(call_id);
    if (call) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "Set userdata for call " + QString::number(call_id)));
        return call->setUserData(data);
    } else {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "setCallUserData: Call doesn't exist!"));
    }
}

//-----------------------------------------------------------------------------
QVariantList JavascriptHandler::getErrorLogData() const
{
    LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "Read error log data"));

    QVariantList log_data;
    QFile file("error.log");
    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);
    while (!in.atEnd()) {
        QVariantMap current;
        Call call;
        in >> call;
        current = call.getInfo();
        log_data << current;
    }

    return log_data;
}

//-----------------------------------------------------------------------------
void JavascriptHandler::deleteErrorLogFile() const
{
    LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "Delete error log file"));

    QFile::remove("error.log");
}

//-----------------------------------------------------------------------------
bool JavascriptHandler::addToConference(const int src_id, const int dst_id) const
{
    Call *call = phone_.getCall(src_id);
    Call *dest_call = phone_.getCall(dst_id);
    if (!call || !dest_call) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Error: one of the selected calls doesn't exist!"));
        return false;
    }
    if (!call->isActive() || !dest_call->isActive()) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Error: one of the selected calls isn't active!"));
        return false;
    }
    if (!call->addToConference(*dest_call)) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Error: failed to connect to source!"));
        return false;
    }
    if (!dest_call->addToConference(*call)) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Error: failed to connect to destination!"));
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
bool JavascriptHandler::removeFromConference(const int src_id, const int dst_id) const
{
    Call *call = phone_.getCall(src_id);
    Call *dest_call = phone_.getCall(dst_id);
    if (!call || !dest_call) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Error: one of the selected calls doesn't exist!"));
        return false;
    }
    if (!call->isActive() || !dest_call->isActive()) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Error: one of the selected calls isn't active!"));
        return false;
    }
    if (call->removeFromConference(*dest_call)) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Error: failed to remove from source!"));
        return false;
    }
    if (dest_call->removeFromConference(*call)) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "Error: failed to remove from destination!"));
        return false;
    }
    return true;
}

//-----------------------------------------------------------------------------
int JavascriptHandler::redirectCall(const int call_id, const QString &dst_uri) const
{
    Call *call = phone_.getCall(call_id);
    if (call) {
        LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_DEBUG, "js_handler", 0, "Redirected call " + QString::number(call_id) + " to " + dst_uri));
        return call->redirect(dst_uri);
    }
    LogHandler::getInstance().logData(LogInfo(LogInfo::STATUS_ERROR, "js_handler", 0, "redirectCall: Call doesn't exist!"));
    return -1;
}

//-----------------------------------------------------------------------------
QVariantList JavascriptHandler::getActiveCallList() const
{
    return phone_.getActiveCallList();
}

//-----------------------------------------------------------------------------
void JavascriptHandler::muteSound(const bool mute, const int call_id) const
{
    phone_.muteSound(mute, call_id);
}

//-----------------------------------------------------------------------------
void JavascriptHandler::muteMicrophone(const bool mute, const int call_id) const
{
    phone_.muteMicrophone(mute, call_id);
}

//-----------------------------------------------------------------------------
QVariantMap JavascriptHandler::getSignalInformation() const
{
    return phone_.getSignalInformation();
}

//-----------------------------------------------------------------------------
QVariant JavascriptHandler::getOption(const QString &name) const
{
    return Config::getInstance().getOption(name);
}

//-----------------------------------------------------------------------------
void JavascriptHandler::setOption(const QString &name, const QVariant &option)
{
    Config::getInstance().setOption(name, option);
    if (name == "url") {
        signalWebPageChanged();
    }
}

//-----------------------------------------------------------------------------
void JavascriptHandler::printPage(const QString &url_str)
{
    QUrl url(url_str);
    signalPrintPage(url);
}

//-----------------------------------------------------------------------------
bool JavascriptHandler::sendLogMessage(const QVariantMap &log) const
{
    QVariant time = log["time"];
    QVariant status = log["status"];
    QVariant domain = log["domain"];
    QVariant code = log["code"];
    QVariant msg = log["message"];

    if (!time.convert(QVariant::String) || !status.convert(QVariant::UInt)
        || !domain.convert(QVariant::String) || !code.convert(QVariant::Int)
        || !msg.convert(QVariant::String))
    {
        return false;
    }

    LogInfo info((LogInfo::Status)status.toUInt(), domain.toString(), code.toInt(), msg.toString());
    info.time_.fromString(time.toString(), "dd.MM.yyyy hh:mm:ss");

    LogHandler::getInstance().logFromJs(info);
    return true;
}

//-----------------------------------------------------------------------------
QStringList JavascriptHandler::getLogFileList() const
{
    return LogHandler::getInstance().getLogFileList();
}

//-----------------------------------------------------------------------------
QString JavascriptHandler::getLogFileContent(const QString &file_name) const
{
    return LogHandler::getInstance().getLogFileContent(file_name);
}

//-----------------------------------------------------------------------------
void JavascriptHandler::deleteLogFile(const QString &file_name) const
{
    LogHandler::getInstance().deleteLogFile(file_name);
}