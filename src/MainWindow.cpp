#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CommentExtractor.h"
#include "CommentSaver.h"
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Connect the openFileButton to its slot
    connect(ui->openFileButton, &QPushButton::clicked, this, &MainWindow::on_openFileButton_clicked);
    // Connect the saveFileButton to its slot
    connect(ui->saveFileButton, &QPushButton::clicked, this, &MainWindow::on_saveFileButton_clicked);

    // Setup the comments table
    ui->commentsTable->setColumnCount(2);
    ui->commentsTable->setHorizontalHeaderLabels({"Line", "Comment"});
    ui->commentsTable->horizontalHeader()->setStretchLastSection(true);
    ui->commentsTable->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openFileButton_clicked()
{
    qDebug() << "on_openFileButton_clicked called";
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Open Code File(s)"),
                                                          QString(),
                                                          tr("Code Files (*.cpp *.h *.py *.ts)"));

    if (!fileNames.isEmpty()) {
        // TODO: Batch upload
        currentFilePath = fileNames.first();
        ui->commentsTable->setRowCount(0); // Clear existing comments
        CommentExtractor extractor;
        QList<QPair<int, QString>> comments = extractor.extractComments(currentFilePath);
        for (const auto &comment : comments) {
            int row = ui->commentsTable->rowCount();
            ui->commentsTable->insertRow(row);
            ui->commentsTable->setItem(row, 0, new QTableWidgetItem(QString::number(comment.first)));
            ui->commentsTable->item(row, 0)->setFlags(ui->commentsTable->item(row, 0)->flags() & ~Qt::ItemIsEditable); // Make line number column non-editable
            ui->commentsTable->setItem(row, 1, new QTableWidgetItem(comment.second));
        }
    }
}

void MainWindow::on_saveFileButton_clicked()
{
    qDebug() << "on_saveFileButton_clicked called";
    if (currentFilePath.isEmpty()) {
        QMessageBox::warning(this, "No File Selected", "Please open a file first before saving.");
        return;
    }

    QList<QPair<int, QString>> modifiedComments;
    for (int row = 0; row < ui->commentsTable->rowCount(); ++row) {
        int lineNumber = ui->commentsTable->item(row, 0)->text().toInt();
        QString comment = ui->commentsTable->item(row, 1)->text();
        modifiedComments.append(qMakePair(lineNumber, comment));
    }

    CommentSaver saver;
    if (saver.saveComments(currentFilePath, modifiedComments)) {
        QMessageBox::information(this, "Save Successful", "Comments saved successfully!");
    } else {
        QMessageBox::critical(this, "Save Failed", "Failed to save comments. Check application logs for details.");
    }
}