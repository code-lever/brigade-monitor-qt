#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTableWidgetItem>
#include <QTimer>
#include <QtNetwork>

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
    void toggleRunning();
    void tableItemsChanged(QTableWidgetItem*);
    void keyTextEdited();
    void doUpdate();
    void updateFinished(QNetworkReply*);
    void sslErrors(QNetworkReply * reply, const QList<QSslError> & errors);

private:
    Ui::MainWindow *ui;
    QMenu *trayIconMenu;
    QAction *startStopAction;
    QSystemTrayIcon *trayIcon;
    QTimer *timer;
    QNetworkAccessManager nam;

    bool isRunning();
    void createTrayIcon();
    void restoreApiKey();
    void restoreMonitoredHosts();
    void storeMonitoredHosts();
};

#endif // MAINWINDOW_H
