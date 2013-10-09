#ifndef WORKERTHREAD_H
#define WORKERTHREAD_H

#include <QList>
#include <QThread>

class WorkerThread : public QThread
{
    Q_OBJECT

public:
    explicit WorkerThread();
    void abort();
    void setFiles(QList<QString> files);

protected:
    virtual void run();

private:
    QList<QString> files;
    bool aborted;

signals:
    void updateQueue(int count);
    void updateCurrentFile(QString filename);
    void updateProgress(int percent);
    void updateTableView(QString filename, QString hash, qint64 size);
};

#endif // WORKERTHREAD_H
