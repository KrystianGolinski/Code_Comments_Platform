#pragma once

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QStyledItemDelegate>
#include <QTextEdit>
#include "CommentExtractor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MultiLineTextDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit MultiLineTextDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

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
    QList<QString> loadedFilePaths;
    QList<QList<CommentGroup>> fileCommentGroups;
    
    QScrollArea *scrollArea_;
    QWidget *scrollWidget_;
    QVBoxLayout *scrollLayout_;
    
    void createFileSection(const QString &filePath, const QList<CommentGroup> &commentGroups, bool addSeparator = false);
    QList<QPair<int, QString>> getModifiedCommentsForFile(int fileIndex);
    QString extractCommentFromFullLine(const QString &fullLine);
    void adjustScrollAreaSizeIntelligently();
};