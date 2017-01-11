#ifndef clientMainWindow_H
#define clientMainWindow_H
#include "ui_cn_client.h"
#include <QKeyEvent>
#include <QMainWindow>
#include <QColor>



namespace Ui {
class clientMainWindow;
}

class clientMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit clientMainWindow(QWidget *parent = 0);
    ~clientMainWindow();

private:
    Ui::clientMainWindow *ui;
    bool eventFilter(QObject *object, QEvent *event);

    int socket_descriptor;
};

#endif // clientMainWindow_H
