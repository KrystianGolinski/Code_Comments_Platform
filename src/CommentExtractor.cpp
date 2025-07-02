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

// Extract comments with full context (for inlines)
QList<QPair<int, QPair<QString, QString>>> CommentExtractor::extractCommentsWithContext(const QString &filePath)
{
    QList<QPair<int, QPair<QString, QString>>> commentsWithContext; // lineNumber, (comment, fullLine)
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open file:" << filePath;
        return commentsWithContext;
    }

    QTextStream in(&file);
    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        lineNumber++;

        // Check for C++ style comments (//)
        QRegularExpression cppSingleLineCommentRegex("//(.*?)$", QRegularExpression::MultilineOption);
        QRegularExpressionMatch cppMatch = cppSingleLineCommentRegex.match(line);
        if (cppMatch.hasMatch()) {
            QString comment = cppMatch.captured(1).trimmed();
            if (!comment.isEmpty()) {
                commentsWithContext.append(qMakePair(lineNumber, qMakePair(comment, line)));
            }
        }

        // Check for Python style comments (#)
        QRegularExpression pythonSingleLineCommentRegex("#(.*?)$", QRegularExpression::MultilineOption);
        QRegularExpressionMatch pythonMatch = pythonSingleLineCommentRegex.match(line);
        if (pythonMatch.hasMatch()) {
            QString comment = pythonMatch.captured(1).trimmed();
            if (!comment.isEmpty()) {
                commentsWithContext.append(qMakePair(lineNumber, qMakePair(comment, line)));
            }
        }

        // Check for C-style multi-line comments (/* */)
        QRegularExpression multiLineCommentRegex("/\\*(.*?)\\*/", QRegularExpression::DotMatchesEverythingOption);
        QRegularExpressionMatch multiMatch = multiLineCommentRegex.match(line);
        if (multiMatch.hasMatch()) {
            QString comment = multiMatch.captured(1).trimmed();
            if (!comment.isEmpty()) {
                commentsWithContext.append(qMakePair(lineNumber, qMakePair(comment, line)));
            }
        }
    }

    file.close();
    return commentsWithContext;
}

QList<CommentGroup> CommentExtractor::extractGroupedComments(const QString &filePath)
{
    QList<CommentGroup> groupedComments;
    QList<QPair<int, QPair<QString, QString>>> allCommentsWithContext = extractCommentsWithContext(filePath);
    
    if (allCommentsWithContext.isEmpty()) {
        return groupedComments;
    }
    
    CommentGroup currentGroup;
    auto firstComment = allCommentsWithContext.first();
    currentGroup.lineNumbers.append(firstComment.first);
    currentGroup.comments.append(firstComment.second.first);
    currentGroup.fullLines.append(firstComment.second.second);
    currentGroup.isInline.append(isInlineComment(firstComment.second.second, firstComment.second.first));
    
    for (int i = 1; i < allCommentsWithContext.size(); ++i) {
        int currentLine = allCommentsWithContext[i].first;
        int previousLine = allCommentsWithContext[i-1].first;
        QString comment = allCommentsWithContext[i].second.first;
        QString fullLine = allCommentsWithContext[i].second.second;
        
        // If current line is consecutive to previous line, add to current group
        if (currentLine == previousLine + 1) {
            currentGroup.lineNumbers.append(currentLine);
            currentGroup.comments.append(comment);
            currentGroup.fullLines.append(fullLine);
            currentGroup.isInline.append(isInlineComment(fullLine, comment));
        } else {
            // Start a new group
            groupedComments.append(currentGroup);
            currentGroup = CommentGroup();
            currentGroup.lineNumbers.append(currentLine);
            currentGroup.comments.append(comment);
            currentGroup.fullLines.append(fullLine);
            currentGroup.isInline.append(isInlineComment(fullLine, comment));
        }
    }
    
    // Don't forget the last group
    if (!currentGroup.lineNumbers.isEmpty()) {
        groupedComments.append(currentGroup);
    }
    
    return groupedComments;
}

bool CommentExtractor::isInlineComment(const QString &fullLine, const QString &comment)
{
    // A comment is inline if there's non-whitespace code before the comment marker
    QString trimmedLine = fullLine.trimmed();
    
    // Check for C++ style comments
    int cppCommentPos = fullLine.indexOf("//");
    if (cppCommentPos > 0) {
        QString beforeComment = fullLine.left(cppCommentPos).trimmed();
        if (!beforeComment.isEmpty()) {
            return true; // There's code before the comment
        }
    }
    
    // Check for Python style comments
    int pythonCommentPos = fullLine.indexOf("#");
    if (pythonCommentPos > 0) {
        QString beforeComment = fullLine.left(pythonCommentPos).trimmed();
        if (!beforeComment.isEmpty()) {
            return true; // There's code before the comment
        }
    }
    
    // Check for C-style comments
    int cStyleCommentPos = fullLine.indexOf("/*");
    if (cStyleCommentPos > 0) {
        QString beforeComment = fullLine.left(cStyleCommentPos).trimmed();
        if (!beforeComment.isEmpty()) {
            return true; // There's code before the comment
        }
    }
    
    return false; // It's a standalone comment
}