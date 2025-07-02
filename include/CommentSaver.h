#pragma once

#include <QObject>
#include <QString>
#include <QList>
#include <QPair>

class CommentSaver : public QObject
{
    Q_OBJECT
public:
    explicit CommentSaver(QObject *parent = nullptr);

    bool saveComments(const QString &filePath, const QList<QPair<int, QString>> &comments);
    bool saveCommentsWithMultiLine(const QString &filePath, const QList<QPair<int, QString>> &comments);

private:
    QString getCommentMarker(const QString &filePath);
    QString getIndentation(const QString &line);
    QStringList expandMultiLineComment(const QString &comment, const QString &marker, const QString &indentation);

signals:

};