#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createTrayIcon();
    trayIcon->show();

    restoreApiKey();
    connect(ui->keyText, SIGNAL(editingFinished()), this, SLOT(keyTextEdited()));

    restoreMonitoredHosts();
    connect(ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tableItemsChanged(QTableWidgetItem*)));
}

MainWindow::~MainWindow()
{
    delete ui;
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
        QString host = settings.value("host").toString();
        QString port = settings.value("port").toString();
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(host));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(port));
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
        QTableWidgetItem *hostItem = ui->tableWidget->item(row, 0);
        QTableWidgetItem *portItem = ui->tableWidget->item(row, 1);

        settings.setArrayIndex(row);
        if (hostItem) { settings.setValue("host", hostItem->text()); }
        if (portItem) { settings.setValue("port", portItem->text()); }
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
    Q_ASSERT(startStopAction);
    startStopAction->setChecked(startStopAction->isChecked());

    QSettings settings;
    settings.setValue("running", startStopAction->isChecked());
}

void MainWindow::keyTextEdited()
{
    Q_ASSERT(ui->keyText);

    ui->keyText->setText(ui->keyText->text().trimmed());

    QSettings settings;
    settings.setValue("api-key", ui->keyText->text());
}
