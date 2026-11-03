
#ifndef EXPENSE_STORAGE_H
#define EXPENSE_STORAGE_H

#include "Expense.h"

#include <QList>
#include <QString>

class ExpenseStorage
{
public:

    explicit ExpenseStorage(const QString& filePath);

    // Load all expenses from JSON
    bool load(
        QList<Expense>& expenses,
        QString* errorMessage = nullptr
    ) const;

    // Save all expenses to JSON
    bool save(
        const QList<Expense>& expenses,
        QString* errorMessage = nullptr
    ) const;

    QString getFilePath() const;

private:

    QString m_filePath;
};

#endif
