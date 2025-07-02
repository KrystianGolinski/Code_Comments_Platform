#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CommentExtractor.h"
#include "CommentSaver.h"
#include <QFileDialog>
#include <QTableWidgetItem>
#include <QDebug>
#include <QMessageBox>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QLabel>
#include <QFrame>
#include <QFileInfo>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Replace the existing table with a scroll area
    QScrollArea *scrollArea = new QScrollArea();
    QWidget *scrollWidget = new QWidget();
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollWidget);
    
    scrollWidget->setLayout(scrollLayout);
    scrollArea->setWidget(scrollWidget);
    scrollArea->setWidgetResizable(true);
    
    // Replace the table in the existing layout
    QVBoxLayout *mainLayout = qobject_cast<QVBoxLayout*>(ui->centralwidget->layout());
    if (mainLayout) {
        // Remove the old table
        mainLayout->removeWidget(ui->commentsTable);
        delete ui->commentsTable;
        
        // Add the scroll area
        mainLayout->addWidget(scrollArea);
        
        // Store references for later use
        scrollArea_ = scrollArea;
        scrollWidget_ = scrollWidget;
        scrollLayout_ = scrollLayout;
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openFileButton_clicked()
{
    static int callCount = 0;
    qDebug() << "on_openFileButton_clicked called - ENTRY #" << ++callCount;
    QStringList fileNames = QFileDialog::getOpenFileNames(this,
                                                          tr("Open Code File(s)"),
                                                          QString(),
                                                          tr("Code Files (*.cpp *.h *.py *.ts)"));

    if (!fileNames.isEmpty()) {
        // Clear existing data
        loadedFilePaths.clear();
        fileCommentGroups.clear();
        
        // Clear the scroll area
        QLayoutItem *child;
        while ((child = scrollLayout_->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        
        CommentExtractor extractor;
        
        // Process each selected file
        for (int i = 0; i < fileNames.size(); ++i) {
            const QString &filePath = fileNames[i];
            loadedFilePaths.append(filePath);
            QList<CommentGroup> commentGroups = extractor.extractGroupedComments(filePath);
            fileCommentGroups.append(commentGroups);
            
            // Create file section
            createFileSection(filePath, commentGroups, i < fileNames.size() - 1);
        }
    }
}

void MainWindow::on_saveFileButton_clicked()
{
    static int saveCallCount = 0;
    qDebug() << "on_saveFileButton_clicked called - ENTRY #" << ++saveCallCount;
    if (loadedFilePaths.isEmpty()) {
        QMessageBox::warning(this, "No Files Selected", "Please open files first before saving.");
        return;
    }

    CommentSaver saver;
    int successCount = 0;
    
    // Save all files
    for (int fileIndex = 0; fileIndex < loadedFilePaths.size(); ++fileIndex) {
        QList<QPair<int, QString>> modifiedComments = getModifiedCommentsForFile(fileIndex);
        
        qDebug() << "File" << fileIndex << ":" << loadedFilePaths[fileIndex];
        qDebug() << "Modified comments count:" << modifiedComments.size();
        for (const auto &comment : modifiedComments) {
            qDebug() << "Line" << comment.first << ":" << comment.second;
        }
        
        if (saver.saveCommentsWithMultiLine(loadedFilePaths[fileIndex], modifiedComments)) {
            successCount++;
        } else {
            qWarning() << "Failed to save:" << loadedFilePaths[fileIndex];
        }
    }
    
    if (successCount == loadedFilePaths.size()) {
        QMessageBox::information(this, "Save Successful", 
            QString("All %1 files saved successfully!").arg(successCount));
    } else {
        QMessageBox::warning(this, "Partial Save", 
            QString("Saved %1 out of %2 files. Check logs for details.")
            .arg(successCount).arg(loadedFilePaths.size()));
    }
}

void MainWindow::createFileSection(const QString &filePath, const QList<CommentGroup> &commentGroups, bool addSeparator)
{
    // File name label
    QFileInfo fileInfo(filePath);
    QLabel *fileLabel = new QLabel(fileInfo.fileName());
    fileLabel->setStyleSheet("font-weight: bold; font-size: 14px; margin: 10px 0px 5px 0px;");
    scrollLayout_->addWidget(fileLabel);
    
    // Comments table for this file
    QTableWidget *table = new QTableWidget();
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels({"Line", "Comment"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
    
    // Hide row numbers/indexes
    table->verticalHeader()->setVisible(false);
    
    // Enable word wrap for better multi-line display
    table->setWordWrap(true);
    
    // Set custom delegate for multi-line editing in the comment column
    MultiLineTextDelegate *delegate = new MultiLineTextDelegate(table);
    table->setItemDelegateForColumn(1, delegate);
    
    // Set table properties
    table->setProperty("fileIndex", loadedFilePaths.size() - 1);
    
    // Populate table with grouped comments
    for (const auto &group : commentGroups) {
        int row = table->rowCount();
        table->insertRow(row);
        
        // Set line range in first column
        table->setItem(row, 0, new QTableWidgetItem(group.getLineRange()));
        table->item(row, 0)->setFlags(table->item(row, 0)->flags() & ~Qt::ItemIsEditable);
        
        // Set combined comments in second column
        table->setItem(row, 1, new QTableWidgetItem(group.getCombinedComments()));
    }
    
    // Set table height to exactly fit content (accounting for multi-line comments)
    int headerHeight = table->horizontalHeader()->height();
    int totalHeight = headerHeight + 2; // Start with header + borders
    
    for (int row = 0; row < table->rowCount(); ++row) {
        // Calculate height needed for multi-line comments
        QString commentText = table->item(row, 1)->text();
        int lineCount = commentText.split('\n').size();
        int rowHeight = std::max(25, lineCount * 20); // Minimum 25px, 20px per line
        table->setRowHeight(row, rowHeight);
        totalHeight += rowHeight;
    }
    
    int tableHeight = totalHeight;
    
    table->setFixedHeight(tableHeight);
    table->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    
    scrollLayout_->addWidget(table);
    
    // Add separator line if requested
    if (addSeparator) {
        QFrame *separator = new QFrame();
        separator->setFrameShape(QFrame::HLine);
        separator->setFrameShadow(QFrame::Sunken);
        separator->setStyleSheet("margin: 15px 0px 10px 0px;");
        scrollLayout_->addWidget(separator);
    }
}

QList<QPair<int, QString>> MainWindow::getModifiedCommentsForFile(int fileIndex)
{
    QList<QPair<int, QString>> modifiedComments;
    
    qDebug() << "Getting modified comments for file index:" << fileIndex;
    qDebug() << "Total layout items:" << scrollLayout_->count();
    
    // Find the table widget for this file
    // Pattern: label, table, separator (except for last file)
    // File 0: positions 0,1,2  File 1: positions 3,4,5  etc.
    // But last file doesn't have separator, so we need to count actual widgets
    int tableIndex = -1;
    int currentFile = 0;
    
    for (int i = 0; i < scrollLayout_->count(); ++i) {
        QLayoutItem *item = scrollLayout_->itemAt(i);
        if (item && item->widget()) {
            QString widgetType = item->widget()->metaObject()->className();
            qDebug() << "Layout item" << i << ":" << widgetType;
            
            QTableWidget *table = qobject_cast<QTableWidget*>(item->widget());
            if (table) {
                qDebug() << "Found table at index" << i << "for file" << currentFile;
                if (currentFile == fileIndex) {
                    tableIndex = i;
                    break;
                }
                currentFile++;
            }
        }
    }
    
    qDebug() << "Selected table index:" << tableIndex;
    
    if (tableIndex < scrollLayout_->count()) {
        QLayoutItem *item = scrollLayout_->itemAt(tableIndex);
        if (item && item->widget()) {
            QTableWidget *table = qobject_cast<QTableWidget*>(item->widget());
            if (table && fileIndex < fileCommentGroups.size()) {
                const QList<CommentGroup> &originalGroups = fileCommentGroups[fileIndex];
                
                for (int row = 0; row < table->rowCount(); ++row) {
                    if (row < originalGroups.size()) {
                        const CommentGroup &originalGroup = originalGroups[row];
                        QString modifiedText = table->item(row, 1)->text();
                        qDebug() << "Row" << row << "original lines:" << originalGroup.lineNumbers;
                        qDebug() << "Row" << row << "modified text:" << modifiedText;
                        QStringList modifiedLines = modifiedText.split('\n');
                        qDebug() << "Row" << row << "split into" << modifiedLines.size() << "lines:" << modifiedLines;
                        
                        // Map each modified comment line back to its original line number
                        // Handle both existing lines and new lines that were added
                        for (int i = 0; i < modifiedLines.size(); ++i) {
                            QString commentToSave;
                            int lineNumber;
                            
                            if (i < originalGroup.lineNumbers.size()) {
                                // This is an existing line being modified
                                lineNumber = originalGroup.lineNumbers[i];
                                
                                // Check if this was an inline comment
                                if (i < originalGroup.isInline.size() && originalGroup.isInline[i]) {
                                    // For inline comments, extract just the comment part from the full line
                                    QString modifiedFullLine = modifiedLines[i];
                                    commentToSave = extractCommentFromFullLine(modifiedFullLine);
                                } else {
                                    // For standalone comments, use the text as-is
                                    commentToSave = modifiedLines[i];
                                }
                            } else {
                                // This is a new line being added after the original group
                                // Insert immediately after the last line of the current group
                                int lastOriginalLine = originalGroup.lineNumbers.last();
                                
                                // Use special notation: -1000 - lineNumber means "insert after this line"
                                lineNumber = -1000 - lastOriginalLine;
                                commentToSave = modifiedLines[i];
                                
                                qDebug() << "Adding new comment line after line" << lastOriginalLine << ":" << commentToSave;
                                qDebug() << "Using special line number" << lineNumber;
                            }
                            
                            modifiedComments.append(qMakePair(lineNumber, commentToSave));
                        }
                    }
                }
            }
        }
    }
    
    return modifiedComments;
}

QString MainWindow::extractCommentFromFullLine(const QString &fullLine)
{
    // Extract just the comment part from a full line of code
    
    // Check for Python style comments (#)
    int pythonCommentPos = fullLine.indexOf("#");
    if (pythonCommentPos >= 0) {
        return fullLine.mid(pythonCommentPos + 1).trimmed();
    }
    
    // Check for C++ style comments (//)
    int cppCommentPos = fullLine.indexOf("//");
    if (cppCommentPos >= 0) {
        return fullLine.mid(cppCommentPos + 2).trimmed();
    }
    
    // Check for C-style comments (/* */)
    int cStyleStart = fullLine.indexOf("/*");
    int cStyleEnd = fullLine.indexOf("*/");
    if (cStyleStart >= 0 && cStyleEnd > cStyleStart) {
        return fullLine.mid(cStyleStart + 2, cStyleEnd - cStyleStart - 2).trimmed();
    }
    
    // If no comment markers found, return the full line (shouldn't happen for inline comments)
    return fullLine.trimmed();
}

// MultiLineTextDelegate implementation
QWidget *MultiLineTextDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    
    QTextEdit *editor = new QTextEdit(parent);
    editor->setAcceptRichText(false); // Plain text only
    editor->setLineWrapMode(QTextEdit::WidgetWidth);
    return editor;
}

void MultiLineTextDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(editor);
    if (textEdit) {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        textEdit->setPlainText(value);
    }
}

void MultiLineTextDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(editor);
    if (textEdit) {
        QString value = textEdit->toPlainText();
        model->setData(index, value, Qt::EditRole);
    }
}

void MultiLineTextDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index)
    
    QTextEdit *textEdit = qobject_cast<QTextEdit*>(editor);
    if (textEdit) {
        QRect rect = option.rect;
        
        // Calculate required height based on content
        QTextDocument *doc = textEdit->document();
        doc->setTextWidth(rect.width() - 10); // Account for margins
        int contentHeight = static_cast<int>(doc->size().height()) + 10; // Add padding
        
        // Set reasonable bounds: minimum 60px, maximum 200px
        int editorHeight = std::max(60, std::min(contentHeight, 200));
        
        rect.setHeight(editorHeight);
        textEdit->setGeometry(rect);
        
        // Ensure the row height matches the editor height
        if (QTableWidget *table = qobject_cast<QTableWidget*>(textEdit->parent()->parent())) {
            table->setRowHeight(index.row(), editorHeight);
        }
    }
}