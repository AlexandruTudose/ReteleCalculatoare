#ifndef CN_CLIENT_H
#define CN_CLIENT_H

#include "ui_cn_client.h"

#include <QKeyEvent>
#include <QMainWindow>
#include <QColor>



#include <thread>

void textUpdate(QTextEdit *text_box);

class clientMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit clientMainWindow(QWidget *parent = 0);
    ~clientMainWindow();

private slots:
    void on_fileList_itemDoubleClicked(QListWidgetItem *item);
    void on_tbbReturn_clicked();
    void on_flbEdit_pressed();
    void on_fileList_itemClicked(QListWidgetItem *item);

private:
    Ui::clientMainWindow *ui;
    bool eventFilter(QObject *object, QEvent *event);

    int socket_descriptor;
    QListWidgetItem *current_item;
    char *current_text;

    void handleKeyEvent(QKeyEvent *keyEvent);

    // Client Logic
    void startTextListener();
    void initFileList();
    void mainFrame();


};

#endif
