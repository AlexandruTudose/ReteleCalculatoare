#include "cn_client.h"

#include "../client-server.h"

clientMainWindow::clientMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::clientMainWindow){
    ui->setupUi(this);
    ui->textBox->installEventFilter(this);

    socket_descriptor = connectToServer();
}

clientMainWindow::~clientMainWindow(){
    endConnection(socket_descriptor);
    delete ui;
}

bool clientMainWindow::eventFilter(QObject *object, QEvent *event)
{
    if (object == ui->textBox && event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key()==Qt::Key_Q){
            // Special tab handling
            char *response;
            requestToServer(socket_descriptor, keyEvent->text().toLatin1().data(), &response);
            qDebug("De la server:$%s$", response);
            qDebug("Efectiv: $%s$",(keyEvent->text().toLatin1().data()));
        }
        return QMainWindow::eventFilter(object, event);

    }
    else{
        return QMainWindow::eventFilter(object, event);
    }
}
