#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_openFileButton_clicked();
    void on_saveFileButton_clicked();

private:
    Ui::MainWindow *ui;
    QString currentFilePath;
};