#ifndef BUDGET_H
#define BUDGET_H

#include <QString>
#include <QJsonObject>

class Budget
{
public:
    Budget() = default;
    Budget(const QString& id, const QString& month, const QString& category,
           double limitAmount);

    QString getId() const;
    QString getMonth() const;
    QString getCategory() const;
    double getLimitAmount() const;

    void setLimitAmount(double amount);
    void setCategory(const QString& category);
    void setMonth(const QString& month);

    bool isValid(QString* errorMessage = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject& object, Budget& budget,
                         QString* errorMessage = nullptr);

private:
    QString id;
    QString month; // yyyy-MM
    QString category;
    double limitAmount = 0.0;
};

#endif
