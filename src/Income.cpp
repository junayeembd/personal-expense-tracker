#include "Income.h"

#include <QUuid>
#include <cmath>

Income::Income(const QString& id, const QDate& date,
               const QString& category, double amount,
               const QString& note)
    : id(id), date(date), category(category.trimmed()),
      amount(amount), note(note.trimmed())
{
}

QString Income::getId() const { return id; }
QDate Income::getDate() const { return date; }
QString Income::getCategory() const { return category; }
double Income::getAmount() const { return amount; }
QString Income::getNote() const { return note; }

void Income::setDate(const QDate& value) { date = value; }
void Income::setCategory(const QString& value) { category = value.trimmed(); }
void Income::setAmount(double value) { amount = value; }
void Income::setNote(const QString& value) { note = value.trimmed(); }

bool Income::isValid(QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    if (id.trimmed().isEmpty()) return fail("Income ID is missing.");
    if (!date.isValid()) return fail("Invalid income date.");
    if (category.trimmed().isEmpty()) return fail("Income source is required.");
    if (!std::isfinite(amount) || amount <= 0.0)
        return fail("Income amount must be greater than zero.");

    if (errorMessage) errorMessage->clear();
    return true;
}

QJsonObject Income::toJson() const
{
    QJsonObject object;
    object.insert("id", id);
    object.insert("date", date.toString(Qt::ISODate));
    object.insert("category", category);
    object.insert("amount", amount);
    object.insert("note", note);
    return object;
}

bool Income::fromJson(const QJsonObject& object, Income& income,
                      QString* errorMessage)
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    QString incomeId = object.value("id").toString().trimmed();
    if (incomeId.isEmpty())
        incomeId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    const QDate incomeDate = QDate::fromString(
        object.value("date").toString(), Qt::ISODate);
    const QString incomeCategory = object.value("category").toString();

    if (!object.value("amount").isDouble())
        return fail("Invalid income amount in JSON data.");

    Income candidate(incomeId, incomeDate, incomeCategory,
                     object.value("amount").toDouble(),
                     object.value("note").toString());

    if (!candidate.isValid(errorMessage)) return false;
    income = candidate;
    if (errorMessage) errorMessage->clear();
    return true;
}
