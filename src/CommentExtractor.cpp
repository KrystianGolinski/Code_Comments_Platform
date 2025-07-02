#include "CommentExtractor.h"
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>
#include <QDebug>

CommentExtractor::CommentExtractor(QObject *parent) : QObject(parent)
{

}

QList<QPair<int, QString>> CommentExtractor::extractComments(const QString &filePath)
{
    QList<QPair<int, QString>> comments;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file:" << filePath;
        return comments;
    }

    QTextStream in(&file);
    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;

        // Regex for C++ style single-line comments (//)
        QRegularExpression cppSingleLineCommentRegex("//(.*?)$", QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator i = cppSingleLineCommentRegex.globalMatch(line);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString comment = match.captured(1).trimmed(); // Capture group 1 for content
            if (!comment.isEmpty()) {
                comments.append(qMakePair(lineNumber, comment));
            }
        }

        // Regex for Python style single-line comments (#)
        QRegularExpression pythonSingleLineCommentRegex("#(.*?)$", QRegularExpression::MultilineOption);
        QRegularExpressionMatchIterator j = pythonSingleLineCommentRegex.globalMatch(line);
        while (j.hasNext()) {
            QRegularExpressionMatch match = j.next();
            QString comment = match.captured(1).trimmed(); // Capture group 1 for content
            if (!comment.isEmpty()) {
                comments.append(qMakePair(lineNumber, comment));
            }
        }

        // Regex for C-style multi-line comments (/* */)
        // Might need to be refined for complex cases
        QRegularExpression multiLineCommentRegex("/\\*(.*?)\\*/", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatchIterator k = multiLineCommentRegex.globalMatch(line);
        while (k.hasNext()) {
            QRegularExpressionMatch match = k.next();
            QString comment = match.captured(1).trimmed(); // Capture group 1 for content
            if (!comment.isEmpty()) {
                comments.append(qMakePair(lineNumber, comment));
            }
        }
    }

    file.close();
    return comments;
}