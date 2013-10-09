#include "window.h"
#include "ui_window.h"

#include <QtDebug>
#include <QClipboard>
#include <QCryptographicHash>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QMimeData>

/**
 * Constructor
 *
 * @brief Window::Window
 * @param parent
 */
Window::Window(QWidget *parent) : QMainWindow(parent), ui(new Ui::Window)
{
    ui->setupUi(this);

    this->setAcceptDrops(true);
    this->setupMenu();
    this->setupButtons();
    this->setupTableView();

    worker = new WorkerThread();
    connect(worker,
            SIGNAL(updateQueue(int)),
            SLOT(onUpdateQueue(int)));
    connect(worker,
            SIGNAL(updateCurrentFile(QString)),
            SLOT(onUpdateCurrentFile(QString)));
    connect(worker,
            SIGNAL(updateProgress(int)),
            SLOT(onUpdateProgress(int)));
    connect(worker,
            SIGNAL(updateTreeView(QString, QString, qint64)),
            SLOT(onUpdateTreeView(QString, QString, qint64)));
}

/**
 * Destructor
 *
 * @brief Window::~Window
 */
Window::~Window()
{
    worker->abort();
    worker->wait();
    delete worker;
    delete model;
    delete ui;
}

/**
 * 绑定菜单动作
 *
 * @brief Window::setupMenu
 */
void Window::setupMenu()
{
    connect(ui->actionOpen,  SIGNAL(triggered()), this, SLOT(open()));
    connect(ui->actionSave,  SIGNAL(triggered()), this, SLOT(save()));
    connect(ui->actionQuit,  SIGNAL(triggered()), this, SLOT(quit()));
    connect(ui->actionCopy,  SIGNAL(triggered()), this, SLOT(copy()));
    connect(ui->actionOntop, SIGNAL(triggered()), this, SLOT(ontop()));
    connect(ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()));
}

/**
 * 绑定按钮动作
 *
 * @brief Window::setupButtons
 */
void Window::setupButtons()
{
    connect(ui->buttonAbort, SIGNAL(clicked()), this, SLOT(doAbort()));
    connect(ui->buttonClear, SIGNAL(clicked()), this, SLOT(doClear()));
}

/**
 * 设置 TableView 列名，行为以及样式等等
 *
 * @brief windows::setupTableView
 */
void Window::setupTableView()
{
    // 设置 TreeView 列名
    model = new QStandardItemModel(this);
    model->setColumnCount(4);
    model->setHeaderData(0, Qt::Horizontal, QStringLiteral("文件"));
    model->setHeaderData(1, Qt::Horizontal, QStringLiteral("摘要"));
    model->setHeaderData(2, Qt::Horizontal, QStringLiteral("字节"));
    model->setHeaderData(3, Qt::Horizontal, QStringLiteral("状态"));
    ui->tableView->setModel(model);

    /** 调整 TableView 外观以及行为 **/
    // 禁止双击编辑数据
    ui->tableView->setEditTriggers(QTableView::NoEditTriggers);

    // 禁止焦点（去掉虚线框）
    ui->tableView->setFocusPolicy(Qt::NoFocus);

    // 设置单元格字体为等宽字体,
    // Windows 默认是 Courier
    QFont font("monospace");
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(8);
    ui->tableView->setFont(font);

    // 整行选择
    ui->tableView->setSelectionBehavior(QTableView::SelectRows);

    // 禁止显示网格
    ui->tableView->setShowGrid(false);

    // 当选中行时，列标题的文字不再自动变为粗体字
    ui->tableView->horizontalHeader()->setHighlightSections(false);

    // 固定行高
    ui->tableView->verticalHeader()->setDefaultSectionSize(18);

    // 行列头的自适应高度
    //ui->tableView->verticalHeader()->resizeSections(QHeaderView::Stretch);

    // 隐藏行列头（默认第一列）
    ui->tableView->verticalHeader()->setVisible(false);

    // ResizeMode
    //ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->tableView->horizontalHeader()->resizeSection(1, 240);

    // 自适应最后一列的宽度
    // ui->tableView->horizontalHeader()->setStretchLastSection(true);
}

/**
 * 打开文件
 *
 * @brief Window::open
 */
void Window::open()
{
    QFileDialog dialog(this);
    dialog.setNameFilter(QStringLiteral("所有文件 (*.*)"));
    dialog.exec();
}

/**
 * 保存
 *
 * @brief Window::save
 */
void Window::save()
{
    int rowCount = model->rowCount();
    if (rowCount == 0) {
        return;
    }

    // 读取 TableView 中的数据
    QStringList data;
    for (int i=0; i<rowCount; i++) {
        data << model->item(i, 1)->text() + "  " + model->item(i, 0)->text();
    }

    // 打开对话框
    QString filename = QFileDialog::getSaveFileName(
                this,
                "Save As",
                QDir::homePath() + "/MD5SUM.md5",
                QStringLiteral("MD5 文件 (*.md5)")
                );

    // 写入文件数据
    if (!filename.isNull()) {
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&file);
            stream << data.join("\n") << endl;
            file.close();;
        }
    }
}

/**
 * 退出
 *
 * @brief Window::quit
 */
void Window::quit()
{
    QApplication::quit();
}

/**
 * 拷贝运行结果
 *
 * @brief Window::copy
 */
void Window::copy()
{
    // 读取鼠标选择的行
    int row;
    QStringList data;
    QModelIndexList indexList = ui->tableView->selectionModel()->selectedRows();
    for (int i=0; i < indexList.count(); ++i) {
        //QMessageBox::information(this,"", QString::number(rows.at(i).row()));
        row = indexList.at(i).row();
        data << (model->item(row, 1)->text() + "  " + model->item(row, 0)->text());
    }

    // 除非得到有效的数据，否则不写入，防止清空剪贴板的原有数据
    if (data.size() > 0) {
        QApplication::clipboard()->setText(data.join("\n"));
    }
}

/**
 * 使窗体总在最前
 *
 * @brief Window::ontop
 */
void Window::ontop()
{
}

/**
 * 显示关于对话框
 *
 * @brief Window::about
 */
void Window::about()
{
    QMessageBox msgbox(this);
    msgbox.setWindowTitle("About");
    msgbox.setTextFormat(Qt::RichText);
    msgbox.setText("QHash v0.1 &copy; 2013 by Verdana &lt;verdana.cn@gmail.com&gt;<br><br>"
                   "For more information please visit:<br>"
                   "<a href='http://github.com/verdana/qhash'>http://github.com/verdana/qhash</a>");
    msgbox.exec();
}


/**
 * 终止校验
 *
 * @brief Window::doAbort
 */
void Window::doAbort()
{
    worker->abort();
    this->onUpdateCurrentFile("");
}

/**
 * 清除结果
 *
 * @brief Window::doClear
 */
void Window::doClear()
{
    // 除非 Worker 已停止，否则不执行下面的动作
    if (worker->isRunning()) {
        return;
    }

    // 当前文件名称清空
    this->onUpdateCurrentFile("");

    // 进度条清零
    ui->progressBar->setValue(0);

    // 清空 TableView
    model->removeRows(0, model->rowCount());
}

/**
 * 更新队列中剩余的文件数量
 *
 * @brief Window::onUpdateQueue
 * @param count
 */
void Window::onUpdateQueue(int count)
{
    ui->labelQueue->setText(QStringLiteral("%1 个文件队列中").arg(QString::number(count)));
}

/**
 * 更新当前正在处理的文件名称
 *
 * @brief Window::onUpdateCurrentFile
 * @param filename
 */
void Window::onUpdateCurrentFile(QString filename)
{
    ui->labelCurrent->setText(QStringLiteral("当前正在处理的文件：%1").arg(filename));
}

/**
 * 更新进度条
 *
 * @brief Window::onUpdateProgress
 * @param num
 */
void Window::onUpdateProgress(int num)
{
    ui->progressBar->setValue(num);
}

/**
 * 更新
 *
 * @brief Window::onUpdateTreeView
 */
void Window::onUpdateTreeView(QString name, QString hash, qint64 size)
{
    QList<QStandardItem*> items;
    items << new QStandardItem(name)
          << new QStandardItem(hash)
          << new QStandardItem(QString::number(size));

    model->appendRow(items);
}

/**
 * 检测拖入的数据是何种类型
 * 这里仅接受文件，放弃其它如文本数据等
 *
 * @brief Window::dragEnterEvent
 * @param e
 */
void Window::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls()) {
        e->acceptProposedAction();
    }
}

/**
 * 拖放完成
 *
 * @brief Window::dropEvent
 */
void Window::dropEvent(QDropEvent *e)
{
    QList<QString> files;
    foreach (const QUrl &url, e->mimeData()->urls()) {
        QString filepath = url.toLocalFile();
        if (false == filepath.isEmpty()) {
            //this->startWorker(filepath);
            files << filepath;
        }
    }
    this->startWorker(files);
}

/**
 * 启动后台线程
 *
 * @brief Window::startWorker
 * @param filepath
 */
void Window::startWorker(QList<QString> files)
{
    worker->setFiles(files);
    worker->start();
}
