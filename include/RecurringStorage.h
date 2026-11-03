#ifndef RECURRING_STORAGE_H
#define RECURRING_STORAGE_H

#include "RecurringRule.h"
#include <QList>
#include <QString>

class RecurringStorage
{
public:
    explicit RecurringStorage(const QString& filePath);
    bool load(QList<RecurringRule>& rules, QString* errorMessage = nullptr) const;
    bool save(const QList<RecurringRule>& rules, QString* errorMessage = nullptr) const;
    QString getFilePath() const;

private:
    QString m_filePath;
};

#endif
