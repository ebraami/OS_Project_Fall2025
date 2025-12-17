#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "ChatMessage.h"
#include "Semaphore.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendClicked();
    void checkIncoming();

private:
    Ui::MainWindow *ui;

    ChatMessage* sharedMsg;
    Semaphore sem;
    QTimer timer;

    bool isProcessA;   // true = A, false = B
};

#endif
