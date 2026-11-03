
#ifndef EXPENSE_H
#define EXPENSE_H

#include <QString>
#include <QDate>
#include <QJsonObject>

class Expense
{
public:

    // Default constructor
    Expense() = default;

    // Parameterized constructor
    Expense(
        const QString& id,
        const QDate& date,
        const QString& category,
        double amount,
        const QString& note
    );

    // Getters
    QString getId() const;
    QDate getDate() const;
    QString getCategory() const;
    double getAmount() const;
    QString getNote() const;

    // Setters
    void setDate(const QDate& date);
    void setCategory(const QString& category);
    void setAmount(double amount);
    void setNote(const QString& note);

    // Validation
    bool isValid(QString* errorMessage = nullptr) const;

    // JSON conversion
    QJsonObject toJson() const;

    static bool fromJson(
        const QJsonObject& object,
        Expense& expense,
        QString* errorMessage = nullptr
    );

private:

    QString id;
    QDate date;
    QString category;
    double amount = 0.0;
    QString note;
};

#endif
