#include "mainwindow.h"
#include "ui_mainwindow.h"

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

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction("Start Monitoring");
    trayIconMenu->addAction("Edit Settings");
    trayIconMenu->addSeparator();
    trayIconMenu->addAction("Exit");

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
}
