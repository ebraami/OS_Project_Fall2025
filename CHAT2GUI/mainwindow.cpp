#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstring>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    isProcessA(true)
{
    ui->setupUi(this);

    sharedMsg = createSharedMemory();

    // Initialize flags (safe even if already set)
    sem.wait();
    sharedMsg->hasAtoB = false;
    sharedMsg->hasBtoA = false;
    sem.signal();

    connect(ui->sendButton, &QPushButton::clicked,
            this, &MainWindow::onSendClicked);

    connect(&timer, &QTimer::timeout,
            this, &MainWindow::checkIncoming);

    // Decide process role
    connect(ui->processSelect, &QComboBox::currentIndexChanged,
            this, [=](int index){
                isProcessA = (index == 0);
                ui->chatView->append(isProcessA ? "[INFO] Process A"
                                                : "[INFO] Process B");
            });

    timer.start(300);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onSendClicked()
{
    QString text = ui->inputLine->text();
    if (text.isEmpty()) return;

    sem.wait();

    if (isProcessA) {
        strncpy(sharedMsg->msgAtoB,
                text.toStdString().c_str(),
                sizeof(sharedMsg->msgAtoB));
        sharedMsg->hasAtoB = true;
        ui->chatView->append("A: " + text);
    } else {
        strncpy(sharedMsg->msgBtoA,
                text.toStdString().c_str(),
                sizeof(sharedMsg->msgBtoA));
        sharedMsg->hasBtoA = true;
        ui->chatView->append("B: " + text);
    }

    sem.signal();
    ui->inputLine->clear();
}

void MainWindow::checkIncoming()
{
    sem.wait();

    if (isProcessA && sharedMsg->hasBtoA) {
        ui->chatView->append("B: " + QString(sharedMsg->msgBtoA));
        sharedMsg->hasBtoA = false;
    }

    if (!isProcessA && sharedMsg->hasAtoB) {
        ui->chatView->append("A: " + QString(sharedMsg->msgAtoB));
        sharedMsg->hasAtoB = false;
    }

    sem.signal();
}
