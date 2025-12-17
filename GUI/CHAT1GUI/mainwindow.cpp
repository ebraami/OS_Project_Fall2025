#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QMetaObject>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isConnected = false;

    ui->statusLabel->setText("Status: Disconnected");
    ui->sendButton->setEnabled(false);
    ui->connectButton->setText("Connect");

    // SEND
    connect(ui->sendButton, &QPushButton::clicked,
            this, &MainWindow::onSendClicked);

    // CONNECT / DISCONNECT
    connect(ui->connectButton, &QPushButton::clicked, this, [this]() {

        if (!isConnected) {
            QString ip = ui->ipInput->text();
            QString portText = ui->portInput->text();
            QString username = ui->usernameInput->text();

            if (ip.isEmpty() || portText.isEmpty() || username.isEmpty()) {
                QMessageBox::warning(this, "Missing info",
                                     "Please enter username, IP and port.");
                return;
            }

            int port = portText.toInt();
            ui->statusLabel->setText("Status: Connecting...");

            if (client.connectTo(ip.toStdString(), port)) {
                isConnected = true;
                ui->statusLabel->setText("Status: Connected");
                ui->connectButton->setText("Disconnect");
                ui->sendButton->setEnabled(true);
                ui->chatView->append("[INFO] Connected to server");
            } else {
                ui->statusLabel->setText("Status: Disconnected");
                QMessageBox::warning(this, "Connection Failed",
                                     "Could not connect to server.");
            }
        }
        else {
            client.disconnect();
            isConnected = false;
            ui->statusLabel->setText("Status: Disconnected");
            ui->connectButton->setText("Connect");
            ui->sendButton->setEnabled(false);
            ui->chatView->append("[INFO] Disconnected");
        }
    });

    // RECEIVE MESSAGE
    client.setOnMessage([this](const std::string& msg) {
        QMetaObject::invokeMethod(this, [this, msg]() {
            ui->chatView->append(QString::fromStdString(msg));
        });
    });

    // HANDLE DISCONNECT
    client.setOnStatus([this](const std::string& status) {
        if (status == "Disconnected") {
            QMetaObject::invokeMethod(this, [this]() {
                isConnected = false;
                ui->statusLabel->setText("Status: Disconnected");
                ui->connectButton->setText("Connect");
                ui->sendButton->setEnabled(false);
                ui->chatView->append("[INFO] Connection lost");
            });
        }
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendClicked()
{
    if (!isConnected) return;

    QString text = ui->messageInput->text();
    QString username = ui->usernameInput->text();

    if (text.isEmpty() || username.isEmpty()) return;

    QString fullMsg = username + ": " + text;

    client.sendMessage(fullMsg.toStdString());
    ui->chatView->append("Me: " + fullMsg);
    ui->messageInput->clear();
}
