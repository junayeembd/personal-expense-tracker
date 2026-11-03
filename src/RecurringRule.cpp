#include "RecurringRule.h"

#include <QUuid>
#include <algorithm>
#include <cmath>

RecurringRule::RecurringRule(const QString& id, const QString& type,
                             const QString& category, double amount,
                             const QString& frequency, const QDate& nextDate,
                             const QString& note, bool active)
    : id(id.trimmed()), type(type.trimmed()), category(category.trimmed()),
      amount(amount), frequency(frequency.trimmed()), nextDate(nextDate),
      monthlyDay(nextDate.isValid() ? nextDate.day() : 1),
      note(note.trimmed()), active(active)
{
}

QString RecurringRule::getId() const { return id; }
QString RecurringRule::getType() const { return type; }
QString RecurringRule::getCategory() const { return category; }
double RecurringRule::getAmount() const { return amount; }
QString RecurringRule::getFrequency() const { return frequency; }
QDate RecurringRule::getNextDate() const { return nextDate; }
QString RecurringRule::getNote() const { return note; }
bool RecurringRule::isActive() const { return active; }

QDate RecurringRule::nextOccurrenceAfter(const QDate& occurrenceDate) const
{
    if (!occurrenceDate.isValid()) return QDate();
    if (frequency == "Weekly") return occurrenceDate.addDays(7);
    if (frequency == "Monthly") {
        const QDate nextMonth = QDate(occurrenceDate.year(), occurrenceDate.month(), 1).addMonths(1);
        if (!nextMonth.isValid()) return QDate();
        const int day = std::min(monthlyDay, nextMonth.daysInMonth());
        return QDate(nextMonth.year(), nextMonth.month(), day);
    }
    if (frequency == "Yearly") return occurrenceDate.addYears(1);
    return QDate();
}

QString RecurringRule::transactionIdForDate(const QDate& occurrenceDate) const
{
    if (id.isEmpty() || !occurrenceDate.isValid()) return QString();
    return QString("recurring-%1-%2").arg(id, occurrenceDate.toString("yyyyMMdd"));
}

void RecurringRule::setType(const QString& value) { type = value.trimmed(); }
void RecurringRule::setCategory(const QString& value) { category = value.trimmed(); }
void RecurringRule::setAmount(double value) { amount = value; }
void RecurringRule::setFrequency(const QString& value) { frequency = value.trimmed(); }
void RecurringRule::setNextDate(const QDate& value) { nextDate = value; }
void RecurringRule::resetScheduleStartDate(const QDate& value)
{
    nextDate = value;
    if (value.isValid()) monthlyDay = value.day();
}
void RecurringRule::setNote(const QString& value) { note = value.trimmed(); }
void RecurringRule::setActive(bool value) { active = value; }

bool RecurringRule::isValid(QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    if (id.isEmpty()) return fail("Recurring rule ID is missing.");
    if (type != "Income" && type != "Expense")
        return fail("Recurring type must be Income or Expense.");
    if (category.isEmpty()) return fail("Recurring category is required.");
    if (!std::isfinite(amount) || amount <= 0.0)
        return fail("Recurring amount must be greater than zero.");
    if (frequency != "Weekly" && frequency != "Monthly" && frequency != "Yearly")
        return fail("Frequency must be Weekly, Monthly, or Yearly.");
    if (!nextDate.isValid()) return fail("Recurring next date is invalid.");
    if (monthlyDay < 1 || monthlyDay > 31) return fail("Recurring monthly day must be between 1 and 31.");

    if (errorMessage) errorMessage->clear();
    return true;
}

QJsonObject RecurringRule::toJson() const
{
    QJsonObject object;
    object.insert("id", id);
    object.insert("type", type);
    object.insert("category", category);
    object.insert("amount", amount);
    object.insert("frequency", frequency);
    object.insert("nextDate", nextDate.toString(Qt::ISODate));
    object.insert("monthlyDay", monthlyDay);
    object.insert("note", note);
    object.insert("active", active);
    return object;
}

bool RecurringRule::fromJson(const QJsonObject& object, RecurringRule& rule,
                             QString* errorMessage)
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    QString ruleId = object.value("id").toString().trimmed();
    if (ruleId.isEmpty())
        ruleId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    if (!object.value("amount").isDouble())
        return fail("Recurring amount is not a valid number.");

    RecurringRule candidate(
        ruleId,
        object.value("type").toString(),
        object.value("category").toString(),
        object.value("amount").toDouble(),
        object.value("frequency").toString(),
        QDate::fromString(object.value("nextDate").toString(), Qt::ISODate),
        object.value("note").toString(),
        object.value("active").toBool(true)
    );

    const QJsonValue savedMonthlyDay = object.value("monthlyDay");
    if (object.contains("monthlyDay") && !savedMonthlyDay.isDouble()) {
        return fail("Recurring monthly day must be a number.");
    }
    if (savedMonthlyDay.isDouble()) {
        candidate.monthlyDay = savedMonthlyDay.toInt();
    }
    if (!candidate.isValid(errorMessage)) return false;
    rule = candidate;
    if (errorMessage) errorMessage->clear();
    return true;
}
