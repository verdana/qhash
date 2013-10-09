#include "workerthread.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QThread>

/**
 * 构造函数
 * @brief WorkerThread::WorkerThread
 */
WorkerThread::WorkerThread()
{
    this->aborted  = false;
}

/**
 * 终止后台工作
 *
 * @brief WorkerThread::abort
 */
void WorkerThread::abort()
{
    this->aborted = true;
}

/**
 * 设定需要校验的文件路径
 *
 * @brief WorkerThread::setFiles
 * @param files
 */
void WorkerThread::setFiles(QList<QString> files)
{
    this->files = files;
}

/**
 * 读取文件并计算 HashValue
 *
 * @brief WorkerThread::run
 */
void WorkerThread::run()
{
    // 排队的文件的数量
    int count = files.size() - 1;
    emit updateQueue(count);

    // 初始化变量
    QString hash, filepath, filename;
    qint64 size, read;
    float percent, lastPercent;

    foreach (filepath, files) {
        //qDebug() << filepath;
        QFile file(filepath);
        if (!file.open(QFile::ReadOnly)) {
            return;
        }
        filename = QFileInfo(filepath).fileName();
        emit updateCurrentFile(filename);

        // 重置进度条
        emit updateProgress(0);

        // 文件大小以及已读取的字节数
        size = file.size();
        read = 0, percent = 0, lastPercent = 0;

        // 如果文件大于 1Mb，拆分读取
        if (size > 1048576) {
            QCryptographicHash crypt(QCryptographicHash::Md5);
            while (!file.atEnd()) {
                // 终止处理？
                if (this->aborted == true) {
                    // qDebug() << "This opertation is aborted.";
                    this->quit();
                    return;
                }

                crypt.addData(file.read(102400));

                // 每读取 0.5% 的数据，通知主线程更新一次界面
                read += 102400;
                percent = (float) read * 200 / size;
                if (percent - lastPercent > 1) {
                    emit updateProgress(percent);
                    lastPercent = percent;
                }
            }
            hash = crypt.result().toHex();
        }

        // 如果文件小于 1Mb，直接读取全部内容
        if (size <= 1048576) {
            hash = QCryptographicHash::hash(file.readAll(), QCryptographicHash::Md5).toHex();
        }

        // 完成一个文件，计数减1
        count--;

        // 更新主界面
        if (count >= 0) {
            emit updateQueue(count);
        }
        emit updateCurrentFile("");
        emit updateProgress(200);
        emit updateTableView(filename, hash, size);
    }
}
