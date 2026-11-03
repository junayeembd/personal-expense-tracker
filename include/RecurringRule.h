#ifndef RECURRING_RULE_H
#define RECURRING_RULE_H

#include <QString>
#include <QDate>
#include <QJsonObject>

class RecurringRule
{
public:
    RecurringRule() = default;
    RecurringRule(const QString& id, const QString& type,
                  const QString& category, double amount,
                  const QString& frequency, const QDate& nextDate,
                  const QString& note, bool active = true);

    QString getId() const;
    QString getType() const;
    QString getCategory() const;
    double getAmount() const;
    QString getFrequency() const;
    QDate getNextDate() const;
    QString getNote() const;
    bool isActive() const;
    QDate nextOccurrenceAfter(const QDate& occurrenceDate) const;
    QString transactionIdForDate(const QDate& occurrenceDate) const;

    void setType(const QString& value);
    void setCategory(const QString& value);
    void setAmount(double value);
    void setFrequency(const QString& value);
    void setNextDate(const QDate& value);
    void resetScheduleStartDate(const QDate& value);
    void setNote(const QString& value);
    void setActive(bool value);

    bool isValid(QString* errorMessage = nullptr) const;
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject& object, RecurringRule& rule,
                         QString* errorMessage = nullptr);

private:
    QString id;
    QString type;
    QString category;
    double amount = 0.0;
    QString frequency;
    QDate nextDate;
    int monthlyDay = 1;
    QString note;
    bool active = true;
};

#endif
