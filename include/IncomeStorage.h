#ifndef INCOME_STORAGE_H
#define INCOME_STORAGE_H

#include "Income.h"
#include <QList>
#include <QString>

class IncomeStorage
{
public:
    explicit IncomeStorage(const QString& filePath);

    bool load(QList<Income>& incomes,
              QString* errorMessage = nullptr) const;
    bool save(const QList<Income>& incomes,
              QString* errorMessage = nullptr) const;
    QString getFilePath() const;

private:
    QString m_filePath;
};

#endif
