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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createTrayIcon()
{
    QSettings settings;

    startStopAction = new QAction(tr("Start Monitoring"), this);
    startStopAction->setCheckable(true);
    startStopAction->setChecked(settings.value("running", false).toBool());
    connect(startStopAction, SIGNAL(triggered()), this, SLOT(toggleRunning()));

    QAction *showAction = new QAction(tr("Edit Settings"), this);
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
}

void MainWindow::addRow()
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
}

void MainWindow::removeSelectedRow()
{
    QItemSelectionModel* model = ui->tableWidget->selectionModel();
    Q_ASSERT(model);

    if (model->hasSelection())
    {
        int row = model->currentIndex().row();
        ui->tableWidget->removeRow(row);
    }
}

void MainWindow::toggleRunning()
{
    Q_ASSERT(startStopAction);
    startStopAction->setChecked(startStopAction->isChecked());

    QSettings settings;
    settings.setValue("running", startStopAction->isChecked());
}
