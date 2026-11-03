#include "Budget.h"

#include <QDate>
#include <QUuid>
#include <cmath>

Budget::Budget(const QString& id, const QString& month,
               const QString& category, double limitAmount)
    : id(id.trimmed()), month(month.trimmed()),
      category(category.trimmed()), limitAmount(limitAmount)
{
}

QString Budget::getId() const { return id; }
QString Budget::getMonth() const { return month; }
QString Budget::getCategory() const { return category; }
double Budget::getLimitAmount() const { return limitAmount; }

void Budget::setLimitAmount(double amount) { limitAmount = amount; }
void Budget::setCategory(const QString& value) { category = value.trimmed(); }
void Budget::setMonth(const QString& value) { month = value.trimmed(); }

bool Budget::isValid(QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    if (id.isEmpty()) return fail("Budget ID is missing.");
    if (QDate::fromString(month + "-01", Qt::ISODate).isValid() == false ||
        month.size() != 7)
        return fail("Budget month must use yyyy-MM format.");
    if (category.isEmpty()) return fail("Budget category is required.");
    if (!std::isfinite(limitAmount) || limitAmount <= 0.0)
        return fail("Budget limit must be greater than zero.");

    if (errorMessage) errorMessage->clear();
    return true;
}

QJsonObject Budget::toJson() const
{
    QJsonObject object;
    object.insert("id", id);
    object.insert("month", month);
    object.insert("category", category);
    object.insert("limit", limitAmount);
    return object;
}

bool Budget::fromJson(const QJsonObject& object, Budget& budget,
                      QString* errorMessage)
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    QString budgetId = object.value("id").toString().trimmed();
    if (budgetId.isEmpty())
        budgetId = QUuid::createUuid().toString(QUuid::WithoutBraces);

    if (!object.value("limit").isDouble())
        return fail("Budget limit is not a valid number.");

    Budget candidate(budgetId,
                     object.value("month").toString(),
                     object.value("category").toString(),
                     object.value("limit").toDouble());
    if (!candidate.isValid(errorMessage)) return false;

    budget = candidate;
    if (errorMessage) errorMessage->clear();
    return true;
}
