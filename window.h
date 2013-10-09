#ifndef WINDOW_H
#define WINDOW_H

#include "workerthread.h"
#include <QMainWindow>
#include <QStandardItemModel>

namespace Ui {
class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window(QWidget *parent = 0);
    ~Window();

private slots:
    // 菜单
    void open();
    void save();
    void quit();
    void copy();
    void ontop();
    void about();

    // 按钮
    void doAbort();
    void doClear();

    // 线程
    void onUpdateQueue(int);
    void onUpdateCurrentFile(QString);
    void onUpdateProgress(int);
    void onUpdateTableView(QString, QString, qint64);

private:
    Ui::Window *ui;
    QStandardItemModel *model;
    WorkerThread *worker;

    // 界面
    void setupMenu();
    void setupButtons();
    void setupTableView();

    // 事件
    void dragEnterEvent(QDragEnterEvent *);
    void dropEvent(QDropEvent *);

    // 其他
    void startWorker(QList<QString>);
};

#endif // WINDOW_H
