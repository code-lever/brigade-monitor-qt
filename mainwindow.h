#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void addRow();
    void removeSelectedRow();

private:
    Ui::MainWindow *ui;
    QMenu* trayIconMenu;
    QSystemTrayIcon* trayIcon;

    void createTrayIcon();
};

#endif // MAINWINDOW_H
