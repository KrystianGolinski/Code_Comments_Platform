#include "CommentSaver.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

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