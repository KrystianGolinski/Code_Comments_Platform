#pragma once

#include <QObject>
#include <QStringList>
#include <QList>
#include <QPair>

class CommentExtractor : public QObject
{
    Q_OBJECT
public:
    explicit CommentExtractor(QObject *parent = nullptr);

    QList<QPair<int, QString>> extractComments(const QString &filePath);

signals:

};