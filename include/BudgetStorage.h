#ifndef BUDGET_STORAGE_H
#define BUDGET_STORAGE_H

#include "Budget.h"
#include <QList>
#include <QString>

class BudgetStorage
{
public:
    explicit BudgetStorage(const QString& filePath);
    bool load(QList<Budget>& budgets, QString* errorMessage = nullptr) const;
    bool save(const QList<Budget>& budgets, QString* errorMessage = nullptr) const;
    QString getFilePath() const;

private:
    QString m_filePath;
};

#endif
