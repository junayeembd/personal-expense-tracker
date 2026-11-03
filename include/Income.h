#ifndef INCOME_H
#define INCOME_H

#include <QString>
#include <QDate>
#include <QJsonObject>

class Income
{
public:
    Income() = default;
    Income(const QString& id, const QDate& date,
           const QString& category, double amount,
           const QString& note);

    QString getId() const;
    QDate getDate() const;
    QString getCategory() const;
    double getAmount() const;
    QString getNote() const;

    void setDate(const QDate& date);
    void setCategory(const QString& category);
    void setAmount(double amount);
    void setNote(const QString& note);

    bool isValid(QString* errorMessage = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject& object, Income& income,
                         QString* errorMessage = nullptr);

private:
    QString id;
    QDate date;
    QString category;
    double amount = 0.0;
    QString note;
};

#endif
