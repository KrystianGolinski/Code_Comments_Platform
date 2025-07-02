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

signals:

};