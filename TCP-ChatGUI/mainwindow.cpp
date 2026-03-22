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
    if (isConnected)
    {
        closesocket(clientSocket);

        if (recvThread.joinable())
            recvThread.join();
    }

    delete ui;
}

void MainWindow::on_connectButton_clicked()
{
    QString username = ui->usernameLine->text();

    if (username.isEmpty()) return;

    clientSocket = connectToServer("127.0.0.1", 8080);

    std::string uname = username.toStdString();
    send(clientSocket, uname.c_str(), uname.length(), 0);

    isConnected = true;

    recvThread = std::thread([this]() {

        receiveMessages(clientSocket, [this](std::string msg) {

            QMetaObject::invokeMethod(this, [this, msg]() {
                ui->chatDisplay->append(QString::fromStdString(msg));
            });

        });

    });

    ui->chatDisplay->append("Connected as: " + username);
}

void MainWindow::on_sendButton_clicked()
{
    if (!isConnected) return;

    QString message = ui->inputLine->text();
    if (message.isEmpty()) return;

    std::string msg = message.toStdString();

    send(clientSocket, msg.c_str(), msg.length(), 0);

    ui->inputLine->clear();
}