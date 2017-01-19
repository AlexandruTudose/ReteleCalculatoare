#include "cn_client.h"

#include "../client-server.h"
#include <QKeyEvent>
#include <QEvent>
#include <QTime>


void delay(int time_in_ms){
    QTime end_time = QTime::currentTime().addMSecs(time_in_ms);
    while(QTime::currentTime() < end_time)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

clientMainWindow::clientMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::clientMainWindow){
    ui->setupUi(this);
    socket_descriptor = connectToServer();
    mainFrame();

}

clientMainWindow::~clientMainWindow(){
    endConnection(socket_descriptor);
    delete ui;
}

bool clientMainWindow::eventFilter(QObject *object, QEvent *event){
    if (object == ui->textBox && event->type() == QEvent::KeyPress){
        QKeyEvent *keyEvent = static_cast<QKeyEvent *> (event);
        handleKeyEvent(keyEvent);
    }
    return QMainWindow::eventFilter(object, event);
}

void clientMainWindow::handleKeyEvent(QKeyEvent *keyEvent){
    if (keyEvent->key()){
//        qDebug("%d, %d, %d, %d, %d, %d, %d, %d, %c, %d",keyEvent->type(), keyEvent->key(),
//               (int) keyEvent->modifiers(), keyEvent->nativeScanCode(),
//               keyEvent->nativeVirtualKey(), keyEvent->nativeModifiers(), keyEvent->count(),
//               *(keyEvent->text().toLatin1().toStdString().c_str()),
//               keyEvent->text().unicode()->toLatin1(), (keyEvent->text().unicode()->toLatin1()));
        textBoxEvent *text_event = new textBoxEvent;
        text_event->count = keyEvent->count();
        text_event->cursor = ui->textBox->textCursor().position();
        text_event->key = keyEvent->key();
        text_event->modifiers = (int) keyEvent->modifiers();
        text_event->native_modifier = keyEvent->nativeModifiers();
        text_event->native_scan_code = keyEvent->nativeScanCode();
        text_event->native_virtual_key = keyEvent->nativeVirtualKey();
        text_event->type = keyEvent->type();
        text_event->unicode = *(keyEvent->text().toLatin1().toStdString().c_str());
        requestToServer(socket_descriptor, C_EVENT);
        //  qDebug("%d %d %d %d", text_event->count, text_event->key, text_event->unicode, text_event->native_modifier);
        writeEvent(socket_descriptor, text_event);
    }
}

void assign(textBoxEvent *event, int t, int k, int m, int n_c, int n_v, int n_m, int co, int u, int cu){
    event->count = co;
    event->cursor = cu;
    event->key = k;
    event->modifiers = m;
    event->native_modifier = n_m;
    event->native_scan_code = n_c;
    event->native_virtual_key = n_v;
    event->type = t;
    event->unicode = u;
}

void updateTextBox(QPlainTextEdit *text_box, textBoxEvent *event){
    QKeyEvent *key_event = new QKeyEvent((QEvent::Type) event->type, event->key,
        (Qt::KeyboardModifier) event->modifiers, event->native_scan_code,
        event->native_virtual_key, event->native_modifier, QString((char *) &(event->unicode)));
    int old_position = text_box->textCursor().position();
    int old_size = strlen(text_box->toPlainText().toLatin1().toStdString().c_str());
    QTextCursor textCursor1 = text_box->textCursor();
    textCursor1.setPosition(event->cursor);
    text_box->setTextCursor(textCursor1);
    QCoreApplication::postEvent(text_box, key_event);
    delay(20);
    if(old_position > event->cursor){
        old_position -= old_size;
        old_position += strlen(text_box->toPlainText().toLatin1().toStdString().c_str());
    }
    QTextCursor textCursor2 = text_box->textCursor();
    textCursor2.setPosition(old_position);
    text_box->setTextCursor(textCursor2);
}

int eventIsNull(textBoxEvent *event, int val = 0){
//    qDebug("EVENT: %d %d %d %d %d %d %d %d %d.", event->count, event->cursor, event->key, event->native_modifier,
//           event->modifiers, event->native_scan_code, event->native_virtual_key, event->type, event->unicode);
    if(event->count == val && event->cursor == val && event->key == val && event->native_modifier == val &&
       event->modifiers == val && event->native_scan_code == val && event->native_virtual_key == val &&
       event->type == val && event->unicode == val)
        return 1;
    return 0;
}


char *getTextFromBox(QPlainTextEdit *text_box){
    char *text = (char *) malloc(strlen(text_box->toPlainText().toStdString().c_str()));
    strcpy(text, text_box->toPlainText().toStdString().c_str());
    return text;
}


int updateCode(QPlainTextEdit *text_box, int socket_descriptor){
    do{
        //qDebug("client got before read, %d ", socket_descriptor);
        textBoxEvent *event = new textBoxEvent;
        readEvent(socket_descriptor, &event);
        //qDebug("%d %d %d %d", event->count, event->key, event->unicode, event->native_modifier);
        if(eventIsNull(event)){
            return 0;
        }
        if(eventIsNull(event, 1)){
            char *text_string = getTextFromBox(text_box);
            qDebug("got 1_ev: %s", text_string);
            requestToServer(socket_descriptor, C_UPDATE);
            writeStrings(socket_descriptor, 1, &text_string);
        }
        else
            updateTextBox(text_box, event);
    }while(true);
    return 0;
}


int repaintTextBox(QPlainTextEdit *text_box){
    do{
        sleep(1);
        text_box->update();
        QCoreApplication::processEvents();
    }while(true);
    return 0;
}

void clientMainWindow::startTextListener(){
    //std::thread *paint_update = new std::thread(&repaintTextBox, ui->textBox);
    //paint_update->detach();
    std::thread *text_update = new std::thread(&updateCode, ui->textBox, socket_descriptor);
    text_update->detach();
}

void clientMainWindow::initFileList(){
    requestToServer(socket_descriptor, C_FILES);
    char **file_names;
    int *number_of_files = new int();
    readStrings(socket_descriptor, number_of_files, &file_names);
    ui->fileList->clear();
    for(int i=0; i<*number_of_files; ++i){
        ui->fileList->addItem(file_names[i]);
    }
}

void clientMainWindow::mainFrame(){
    setWindowTitle("Collaborative Notepad 2.0");
    ui->editFrame->hide();
    ui->mainMenu->show();
    initFileList();
    on_lflbRefresh_pressed();
}

void clientMainWindow::on_fileList_itemDoubleClicked(QListWidgetItem *item){
    char *request = (char *) malloc(strlen(C_EDIT) + item->text().length() + 1);
    sprintf(request, "%s %s", C_EDIT, item->text().toLatin1().toStdString().c_str());
    requestToServer(socket_descriptor, request, &current_text);
    //qDebug("request snet : %s", current_text);
    setWindowTitle(windowTitle() + " - " + item->text());
    ui->mainMenu->hide();
    ui->textBox->clear();
    ui->textBox->appendPlainText(current_text);
    ui->editFrame->show();
    startTextListener();
    ui->textBox->installEventFilter(this);
}

void clientMainWindow::on_tbbReturn_clicked(){
    requestToServer(socket_descriptor, C_NULL_EVENT);
    char *text_string = getTextFromBox(ui->textBox);
    writeStrings(socket_descriptor, 1, &text_string);
    ui->textBox->removeEventFilter(this);
    mainFrame();
}

void clientMainWindow::on_flbEdit_pressed(){
    if(current_item)
        on_fileList_itemDoubleClicked(current_item);
}

void clientMainWindow::on_fileList_itemClicked(QListWidgetItem *item){
    current_item = item;
}

void clientMainWindow::on_flbDownload_pressed(){
    const char* FILES_DIR = "Files";
    if(current_item){
        char *request = (char *) malloc(strlen(C_EDIT) + current_item->text().length() + 1);
        const char *file_name = current_item->text().toLatin1().toStdString().c_str();
        sprintf(request, "%s %s", C_EDIT, file_name);
        requestToServer(socket_descriptor, request, &current_text);
        char *path = (char *) malloc(strlen(file_name)+strlen(FILES_DIR)+1);
        sprintf(path, "%s/%s", FILES_DIR, file_name);
        FILE *f = fopen(path, "wb");
        fprintf(f, "%s", current_text);
        fclose(f);
        on_lflbRefresh_pressed();
    }
}

void clientMainWindow::on_flbNew_pressed(){

    if(ui->lineEdit->text().toStdString().c_str() != NULL){
        int length = strlen(ui->lineEdit->text().toStdString().c_str()) + strlen(C_NEW) + 2;
        char *request = (char *) malloc (length);
        memset(request, 0, length);
        sprintf(request, "%s %s", C_NEW,ui->lineEdit->text().toStdString().c_str());
        requestToServer(socket_descriptor, request);
    }
    initFileList();
}


my_files *getFileNames(){
    const char* FILES_DIR = "Files";
    my_files *server_files = new my_files();
    server_files->number_of_files = 0;
    struct dirent *p_dirent;
    DIR *p_dir = opendir(FILES_DIR);
    //qDebug("%d", errno);
    //if(errno) return server_files;
    while((p_dirent = readdir(p_dir))){
        if(strcmp(p_dirent->d_name, (char *) ".") && strcmp(p_dirent->d_name, (char *) "..")){
            qDebug("got here");
            server_files->file_names = (char **) realloc(server_files->file_names, (server_files->number_of_files + 1) * sizeof(char *));
            (server_files->file_names)[(server_files->number_of_files)++] = (char *) malloc(strlen(p_dirent->d_name) + 1);
            memset((server_files->file_names)[(server_files->number_of_files) - 1], 0, strlen(p_dirent->d_name) + 1);
            strcpy(server_files->file_names[server_files->number_of_files - 1], p_dirent->d_name);
        }
    }
    return server_files;
}

void clientMainWindow::on_lflbRefresh_pressed(){
    ui->localFileList->clear();
    my_files *client_files = getFileNames();
    qDebug("%d", client_files->number_of_files);
    for(int i=0; i<client_files->number_of_files; ++i){
        ui->localFileList->addItem(client_files->file_names[i]);
    }
}

void clientMainWindow::on_flbRefresh_pressed(){
    initFileList();
}

void clientMainWindow::on_localDelete_pressed(){
    if(current_local_item){
        const char* FILES_DIR = "Files";
        const char *file_name = current_local_item->text().toStdString().c_str();
        char *path = (char *) malloc (strlen(FILES_DIR) + strlen(file_name) +2);
        memset(path, 0, strlen(FILES_DIR) + strlen(file_name) +2);
        sprintf(path, "%s/%s", FILES_DIR, file_name);
        remove(path);
    }
    on_lflbRefresh_pressed();
}

void clientMainWindow::on_serverDelete_pressed(){
    if(current_item){
        const char *file_name = current_item->text().toStdString().c_str();
        char *buffer = (char *) malloc (strlen(file_name) + 2 + strlen(C_REMOVE));
        memset(buffer, 0, strlen(C_REMOVE) + strlen(file_name) + 2);
        sprintf(buffer, "%s %s", C_REMOVE, file_name);
        qDebug("%s", buffer);
        requestToServer(socket_descriptor, buffer);
        initFileList();
    }
}

void clientMainWindow::on_localFileList_itemClicked(QListWidgetItem *item){
    current_local_item = item;
}
