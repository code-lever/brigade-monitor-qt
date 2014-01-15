#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
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
