#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMetaObject>
#include <QString>
#include <winsock2.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    // clean shutdown
    if (isConnected)
    {
        closesocket(clientSocket);

        if (recvThread.joinable())
            recvThread.join();
    }

    delete ui;
}

/*
    CONNECT BUTTON
    - reads username
    - connects to server
    - starts receive thread
*/
void MainWindow::on_connectButton_clicked()
{
    QString username = ui->usernameLine->text();
    if (username.isEmpty()) return;

    clientSocket = connectToServer("127.0.0.1", 8080);

    std::string uname = username.toStdString();

    // first thing server expects → username
    send(clientSocket, uname.c_str(), uname.length(), 0);

    isConnected = true;

    // start background thread for receiving messages
    recvThread = std::thread([this]() {

        receiveMessages(clientSocket, [this](std::string msg) {

            QMetaObject::invokeMethod(this, [this, msg]() {

                QString qmsg = QString::fromStdString(msg);

                // -------- USER LIST RESPONSE --------
                if (qmsg.startsWith("Online Users:")) {
                    updateUserList(qmsg);
                    return;
                }

                // -------- JOIN / LEAVE EVENTS --------
                if (qmsg.contains("joined") || qmsg.contains("left")) {
                    ui->chatDisplay->append(qmsg);

                    // ask server again for updated list
                    std::string listCmd = "/list";
                    send(clientSocket, listCmd.c_str(), listCmd.length(), 0);
                    return;
                }

                // -------- NORMAL MESSAGE FORMATTING --------
                // Expected format:
                // [12:30] Anup: Hello

                QString time = qmsg.section(' ', 0, 0);
                QString user = qmsg.section(' ', 1, 1).replace(":", "");
                QString text = qmsg.section(' ', 2);

                QString formatted =
                    "<div style='margin:6px;'>"
                    "<span style='color:#888;'>" + time + "</span> "
                             "<b>" + user + "</b>: "
                    + text +
                    "</div>";

                ui->chatDisplay->append(formatted);

            });

        });

    });

    ui->chatDisplay->append("Connected as: " + username);

    // request user list immediately after connecting
    std::string listCmd = "/list";
    send(clientSocket, listCmd.c_str(), listCmd.length(), 0);
}

/*
    SEND BUTTON
    - sends message to server
*/
void MainWindow::on_sendButton_clicked()
{
    if (!isConnected) return;

    QString message = ui->inputLine->text();
    if (message.isEmpty()) return;

    std::string msg = message.toStdString();
    send(clientSocket, msg.c_str(), msg.length(), 0);

    ui->inputLine->clear();
}

/*
    PARSE USER LIST
    Format received:
    Online Users:
    - Anup
    - Kanak
*/
void MainWindow::updateUserList(const QString &message)
{
    ui->userList->clear();

    QStringList lines = message.split("\n");

    for (const QString &line : lines) {
        if (line.startsWith("- ")) {
            QString username = line.mid(2);
            ui->userList->addItem(username);
        }
    }
}