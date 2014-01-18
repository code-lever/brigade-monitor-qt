#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "updater.h"

#include <QDebug>
#include <QSettings>
#include <QTimer>

#define UPDATE_PERIOD 60000

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow), trayIconMenu(0), startStopAction(0),
    trayIcon(0), timer(0), nam()
{
    ui->setupUi(this);
    setFixedSize(geometry().width(), geometry().height());
    setWindowFlags(Qt::Window | Qt::WindowTitleHint | Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint);

    connect(&nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateFinished(QNetworkReply*)));
    connect(&nam, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));

    createTrayIcon();
    trayIcon->show();

    restoreApiKey();
    connect(ui->keyText, SIGNAL(editingFinished()), this, SLOT(keyTextEdited()));

    restoreMonitoredHosts();
    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tableItemsChanged(QTableWidgetItem*)));

    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, SIGNAL(timeout()), this, SLOT(doUpdate()));

    if (isRunning())
    {
        timer->start(UPDATE_PERIOD);
    }

    qDebug() << QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::isRunning()
{
    return startStopAction && startStopAction->isChecked();
}

void MainWindow::createTrayIcon()
{
    QSettings settings;

    startStopAction = new QAction(tr("&Start Monitoring"), this);
    startStopAction->setCheckable(true);
    startStopAction->setChecked(settings.value("running", false).toBool());
    connect(startStopAction, SIGNAL(triggered()), this, SLOT(toggleRunning()));

    QAction *showAction = new QAction(tr("&Edit Settings"), this);
    connect(showAction, SIGNAL(triggered()), this, SLOT(show()));

    QAction *exitAction = new QAction(tr("&Quit"), this);
    connect(exitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(startStopAction);
    trayIconMenu->addAction(showAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(exitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/images/icon.png"));
}

void MainWindow::restoreApiKey()
{
    Q_ASSERT(ui->keyText);

    QSettings settings;
    ui->keyText->setText(settings.value("api-key").toString());
}

void MainWindow::restoreMonitoredHosts()
{
    Q_ASSERT(ui->tableWidget);

    QSettings settings;
    int size = settings.beginReadArray("monitored-hosts");
    ui->tableWidget->setRowCount(size);
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        QString host = settings.value("host").toString();
        QString port = settings.value("port").toString();
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(name));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(host));
        ui->tableWidget->setItem(i, 2, new QTableWidgetItem(port));
    }
    settings.endArray();
}

void MainWindow::storeMonitoredHosts()
{
    Q_ASSERT(ui->tableWidget);

    QSettings settings;
    settings.remove("monitored-hosts");
    settings.beginWriteArray("monitored-hosts");

    int rows = ui->tableWidget->rowCount();
    for (int row = 0; row < rows; ++row)
    {
        QTableWidgetItem *nameItem = ui->tableWidget->item(row, 0);
        QTableWidgetItem *hostItem = ui->tableWidget->item(row, 1);
        QTableWidgetItem *portItem = ui->tableWidget->item(row, 2);

        settings.setArrayIndex(row);
        if (nameItem) { settings.setValue("name", nameItem->text().trimmed()); }
        if (hostItem) { settings.setValue("host", hostItem->text().trimmed()); }
        if (portItem) { settings.setValue("port", portItem->text().trimmed()); }
    }
    settings.endArray();
}

void MainWindow::addRow()
{
    Q_ASSERT(ui->tableWidget);

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
}

void MainWindow::removeSelectedRow()
{
    Q_ASSERT(ui->tableWidget);

    QItemSelectionModel* model = ui->tableWidget->selectionModel();
    Q_ASSERT(model);

    if (model->hasSelection())
    {
        int row = model->currentIndex().row();
        ui->tableWidget->removeRow(row);
        storeMonitoredHosts();
    }
}

void MainWindow::tableItemsChanged(QTableWidgetItem*)
{
    storeMonitoredHosts();
}

void MainWindow::toggleRunning()
{
    QSettings settings;
    settings.setValue("running", isRunning());

    if (isRunning())
    {
        timer->start(5000);
    }
    else
    {
        timer->stop();
    }
}

void MainWindow::keyTextEdited()
{
    Q_ASSERT(ui->keyText);

    ui->keyText->setText(ui->keyText->text().trimmed());

    QSettings settings;
    settings.setValue("api-key", ui->keyText->text());
}

void MainWindow::doUpdate()
{
    QString token;

    {
        QSettings settings;
        token = settings.value("api-key").toString();

        if (token.isEmpty())
        {
            qDebug() << "Token empty, skipping update...";
            return;
        }
    }

    try
    {
        QJsonDocument updates = Updater(this).update();

#if 0
        QUrl url("http://localhost:3000/api/v1/hosts");
#else
        QUrl url("https://app.brigade.io/api/v1/hosts");
#endif
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

        QUrlQuery query;
        query.addQueryItem("token", token);
        query.addQueryItem("updates", updates.toJson());

        QByteArray data;
        data.append(query.query(QUrl::FullyEncoded));

        qDebug() << "About to post" << data.length() << "bytes to" << url;
        nam.post(request, data);
    }
    catch (std::exception& e)
    {
        qDebug() << "Exception submitting updates:" << e.what();
    }

    if (isRunning())
    {
        timer->start(UPDATE_PERIOD);
    }
}

void MainWindow::updateFinished(QNetworkReply* reply)
{
    Q_ASSERT(reply);

    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (401 == status)
    {
        // XXX get this info to user...
        qDebug() << "Unauthorized response submitting updates, check key!";
        return;
    }

    qDebug() << QString("Submitted updates (status: %1)").arg(status);
}

void MainWindow::sslErrors(QNetworkReply * reply, const QList<QSslError> & errors)
{
    // XXX kill this once ssl cert is setup
    qDebug() << "ssl errors:" << errors;
    reply->ignoreSslErrors(errors);
}
