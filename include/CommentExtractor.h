#pragma once

#include <QObject>
#include <QStringList>
#include <QList>
#include <QPair>

struct CommentGroup {
    QList<int> lineNumbers;
    QStringList comments;
    QStringList fullLines; // Store full line context for inline comments
    QList<bool> isInline;  // Track which comments are inline
    
    QString getLineRange() const {
        if (lineNumbers.size() == 1) {
            return QString::number(lineNumbers.first());
        }
        QStringList lineStrings;
        for (int line : lineNumbers) {
            lineStrings.append(QString::number(line));
        }
        return lineStrings.join(",");
    }
    
    QString getCombinedComments() const {
        QStringList displayComments;
        for (int i = 0; i < comments.size(); ++i) {
            if (i < isInline.size() && isInline[i] && i < fullLines.size()) {
                // For inline comments, show the full line without leading whitespace
                displayComments.append(fullLines[i].trimmed());
            } else {
                // For standalone comments, show just the comment
                displayComments.append(comments[i]);
            }
        }
        return displayComments.join("\n");
    }
};

class CommentExtractor : public QObject
{
    Q_OBJECT
public:
    explicit CommentExtractor(QObject *parent = nullptr);

    QList<QPair<int, QString>> extractComments(const QString &filePath);
    QList<QPair<int, QPair<QString, QString>>> extractCommentsWithContext(const QString &filePath);
    QList<CommentGroup> extractGroupedComments(const QString &filePath);

private:
    bool isInlineComment(const QString &fullLine, const QString &comment);

signals:

};