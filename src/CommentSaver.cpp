#include "CommentSaver.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>
#include <QFileInfo>
#include <algorithm>

CommentSaver::CommentSaver(QObject *parent) : QObject(parent)
{

}

bool CommentSaver::saveComments(const QString &filePath, const QList<QPair<int, QString>> &comments)
{
    QFile originalFile(filePath);
    if (!originalFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open original file for reading:" << filePath;
        return false;
    }

    QString tempFilePath = filePath + ".tmp";
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Could not open temporary file for writing:" << tempFilePath;
        originalFile.close();
        return false;
    }

    QTextStream in(&originalFile);
    QTextStream out(&tempFile);

    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;

        // Check if this line number has a modified comment
        for (const auto &commentPair : comments) {
            if (commentPair.first == lineNumber) {
                // Regex for C++ style single-line comments (//)
                // Captures the comment marker and leading whitespace in group 1, and the comment content in group 2
                QRegularExpression cppSingleLineCommentRegex("^(\\s*//\\s*)(.*)$", QRegularExpression::MultilineOption);
                if (cppSingleLineCommentRegex.match(line).hasMatch()) {
                    line.replace(cppSingleLineCommentRegex, "\\1" + commentPair.second);
                }
                // Regex for Python style single-line comments (#)
                // Captures the comment marker and leading whitespace in group 1, and the comment content in group 2
                else {
                    QRegularExpression pythonSingleLineCommentRegex("^(\\s*#\\s*)(.*)$", QRegularExpression::MultilineOption);
                    if (pythonSingleLineCommentRegex.match(line).hasMatch()) {
                        line.replace(pythonSingleLineCommentRegex, "\\1" + commentPair.second);
                    }
                }
                // Note: Multi-line comments (/* */) are not handled for editing in this version.
                break; // Assumption: one relevant comment per line for replacement
            }
        }
        out << line << "\n";
    }

    originalFile.close();
    tempFile.close();

    // Replace the original file with the temporary file
    if (originalFile.remove()) {
        if (tempFile.rename(filePath)) {
            qDebug() << "Comments saved successfully to:" << filePath;
            return true;
        } else {
            qWarning() << "Could not rename temporary file to original:" << tempFilePath;
            return false;
        }
    } else {
        qWarning() << "Could not remove original file:" << filePath;
        return false;
    }
}

bool CommentSaver::saveCommentsWithMultiLine(const QString &filePath, const QList<QPair<int, QString>> &comments)
{
    QFile originalFile(filePath);
    if (!originalFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open original file for reading:" << filePath;
        return false;
    }

    // Read all lines into memory
    QStringList allLines;
    QTextStream in(&originalFile);
    while (!in.atEnd()) {
        allLines.append(in.readLine());
    }
    originalFile.close();

    QString commentMarker = getCommentMarker(filePath);
    
    // Process comments in reverse order to handle line number shifts correctly
    QList<QPair<int, QString>> sortedComments = comments;
    std::sort(sortedComments.begin(), sortedComments.end(), 
              [](const QPair<int, QString> &a, const QPair<int, QString> &b) {
                  return a.first > b.first; // Sort in descending order
              });

    for (const auto &commentPair : sortedComments) {
        int lineNumber = commentPair.first;
        QString newComment = commentPair.second;
        
        qDebug() << "Processing comment for line" << lineNumber << ":" << newComment;
        
        // Handle special line number signals
        if (lineNumber < -1000) {
            // Insert after line: -1000 - lineNumber means "insert after this line"
            int insertAfterLine = -1000 - lineNumber;
            int insertIndex = insertAfterLine; // Insert after means at position lineNumber+1 (0-based: lineNumber)
            
            qDebug() << "Insert after line" << insertAfterLine << "(index" << insertIndex << ")";
            
            QString commentMarkerLine = commentMarker + " " + newComment;
            if (insertIndex <= allLines.size()) {
                allLines.insert(insertIndex, commentMarkerLine);
                qDebug() << "Inserted new line after line" << insertAfterLine << ":" << commentMarkerLine;
            }
            continue; // Skip normal processing
        } else if (lineNumber < 0) {
            // Insert before line: negative lineNumber means "insert before this line"
            int insertBeforeLine = -lineNumber;
            int insertIndex = insertBeforeLine - 1; // Convert to 0-based
            
            qDebug() << "Insert before line" << insertBeforeLine << "(index" << insertIndex << ")";
            
            QString commentMarkerLine = commentMarker + " " + newComment;
            if (insertIndex <= allLines.size()) {
                allLines.insert(insertIndex, commentMarkerLine);
                qDebug() << "Inserted new line before existing line:" << commentMarkerLine;
            }
            continue; // Skip normal processing
        }
        
        int lineIndex = lineNumber - 1; // Convert to 0-based index
        qDebug() << "File has" << allLines.size() << "lines, trying to access line index" << lineIndex;
        
        if (lineIndex >= 0 && lineIndex < allLines.size()) {
            QString originalLine = allLines[lineIndex];
            QString indentation = getIndentation(originalLine);
            
            // Check if the comment spans multiple lines
            QStringList commentLines = newComment.split('\n');
            
            if (commentLines.size() == 1) {
                // Single line comment - update existing line
                QRegularExpression cppRegex("^(\\s*//\\s*)(.*)$");
                QRegularExpression pythonRegex("^(\\s*#\\s*)(.*)$");
                QRegularExpression inlineRegex("^(.+)(//|#)(.*)$");
                
                if (cppRegex.match(originalLine).hasMatch()) {
                    allLines[lineIndex] = originalLine.replace(cppRegex, "\\1" + newComment);
                    qDebug() << "Replaced C++ comment:" << allLines[lineIndex];
                } else if (pythonRegex.match(originalLine).hasMatch()) {
                    allLines[lineIndex] = originalLine.replace(pythonRegex, "\\1" + newComment);
                    qDebug() << "Replaced Python comment:" << allLines[lineIndex];
                } else if (inlineRegex.match(originalLine).hasMatch()) {
                    // Handle inline comments
                    QRegularExpressionMatch match = inlineRegex.match(originalLine);
                    QString codePart = match.captured(1);
                    QString marker = match.captured(2);
                    allLines[lineIndex] = codePart + marker + " " + newComment;
                    qDebug() << "Replaced inline comment:" << allLines[lineIndex];
                } else {
                    // Line exists but has no comment - insert new comment
                    QString newCommentLine = indentation + commentMarker + " " + newComment;
                    allLines[lineIndex] = newCommentLine;
                    qDebug() << "Inserted new comment on existing line:" << newCommentLine;
                }
            } else {
                // Multi-line comment - replace original and insert new lines
                QStringList expandedLines = expandMultiLineComment(newComment, commentMarker, indentation);
                
                // Replace the original line with the first comment line
                allLines[lineIndex] = expandedLines.first();
                
                // Insert additional lines after the original
                for (int i = 1; i < expandedLines.size(); ++i) {
                    allLines.insert(lineIndex + i, expandedLines[i]);
                }
            }
        } else {
            // This is a new line to be inserted - push everything down
            qDebug() << "Inserting completely new line at position" << commentPair.first;
            
            QString commentMarkerLine = commentMarker + " " + newComment;
            
            // Insert at the correct position, pushing existing lines down
            if (lineIndex <= allLines.size()) {
                allLines.insert(lineIndex, commentMarkerLine);
            } else {
                // Beyond file size - add empty lines first, then the comment
                while (allLines.size() < lineIndex) {
                    allLines.append("");
                }
                allLines.append(commentMarkerLine);
            }
            
            qDebug() << "Inserted new line:" << commentMarkerLine << "at index" << lineIndex;
        }
    }

    // Write the modified content back to file
    QString tempFilePath = filePath + ".tmp";
    QFile tempFile(tempFilePath);
    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        qWarning() << "Could not open temporary file for writing:" << tempFilePath;
        return false;
    }

    QTextStream out(&tempFile);
    for (const QString &line : allLines) {
        out << line << "\n";
    }
    tempFile.close();

    // Replace the original file
    QFile originalFileForRemoval(filePath);
    if (originalFileForRemoval.remove()) {
        if (tempFile.rename(filePath)) {
            qDebug() << "Multi-line comments saved successfully to:" << filePath;
            return true;
        } else {
            qWarning() << "Could not rename temporary file:" << tempFilePath;
            return false;
        }
    } else {
        qWarning() << "Could not remove original file:" << filePath;
        return false;
    }
}

QString CommentSaver::getCommentMarker(const QString &filePath)
{
    QString extension = QFileInfo(filePath).suffix().toLower();
    
    if (extension == "py") {
        return "#";
    } else if (extension == "cpp" || extension == "h" || extension == "ts" || extension == "js") {
        return "//";
    }
    
    return "#"; // Default to Python style
}

QString CommentSaver::getIndentation(const QString &line)
{
    QString indentation;
    for (const QChar &ch : line) {
        if (ch == ' ' || ch == '\t') {
            indentation += ch;
        } else {
            break;
        }
    }
    return indentation;
}

QStringList CommentSaver::expandMultiLineComment(const QString &comment, const QString &marker, const QString &indentation)
{
    QStringList lines = comment.split('\n');
    QStringList result;
    
    for (const QString &line : lines) {
        if (!line.trimmed().isEmpty()) {
            result.append(indentation + marker + " " + line.trimmed());
        } else {
            result.append(indentation + marker);
        }
    }
    
    return result;
}