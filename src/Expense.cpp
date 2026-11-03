
#include "Expense.h"

#include <QUuid>
#include <cmath>

// Constructor
Expense::Expense(
    const QString& id,
    const QDate& date,
    const QString& category,
    double amount,
    const QString& note
)
    : id(id),
      date(date),
      category(category),
      amount(amount),
      note(note)
{
}

// Getters

QString Expense::getId() const
{
    return id;
}

QDate Expense::getDate() const
{
    return date;
}

QString Expense::getCategory() const
{
    return category;
}

double Expense::getAmount() const
{
    return amount;
}

QString Expense::getNote() const
{
    return note;
}

// Setters

void Expense::setDate(const QDate& date)
{
    this->date = date;
}

void Expense::setCategory(const QString& category)
{
    this->category = category.trimmed();
}

void Expense::setAmount(double amount)
{
    this->amount = amount;
}

void Expense::setNote(const QString& note)
{
    this->note = note.trimmed();
}

// Validate expense
bool Expense::isValid(QString* errorMessage) const
{
    auto setError = [&](const QString& message) {
        if (errorMessage)
            *errorMessage = message;
    };

    if (id.trimmed().isEmpty()) {
        setError("Expense ID is missing.");
        return false;
    }

    if (!date.isValid()) {
        setError("Invalid expense date.");
        return false;
    }

    if (category.trimmed().isEmpty()) {
        setError("Expense category is required.");
        return false;
    }

    if (!std::isfinite(amount) || amount <= 0) {
        setError("Amount must be greater than zero.");
        return false;
    }

    setError("");
    return true;
}

// Convert Expense to JSON
QJsonObject Expense::toJson() const
{
    QJsonObject object;

    object.insert("id", id);
    object.insert("date", date.toString(Qt::ISODate));
    object.insert("category", category);
    object.insert("amount", amount);
    object.insert("note", note);

    return object;
}

// Load Expense from JSON
bool Expense::fromJson(
    const QJsonObject& object,
    Expense& expense,
    QString* errorMessage
)
{
    auto setError = [&](const QString& message) {
        if (errorMessage)
            *errorMessage = message;
    };

    QString expenseId = object.value("id").toString();

    // Support Version 1 data without an ID
    if (expenseId.trimmed().isEmpty()) {
        expenseId = QUuid::createUuid().toString(
            QUuid::WithoutBraces
        );
    }

    QDate expenseDate = QDate::fromString(
        object.value("date").toString(),
        Qt::ISODate
    );

    QString expenseCategory =
        object.value("category").toString();

    if (!object.value("amount").isDouble()) {
        setError("Invalid expense amount.");
        return false;
    }

    double expenseAmount =
        object.value("amount").toDouble();

    QString expenseNote =
        object.value("note").toString();

    Expense loadedExpense(
        expenseId,
        expenseDate,
        expenseCategory,
        expenseAmount,
        expenseNote
    );

    if (!loadedExpense.isValid(errorMessage)) {
        return false;
    }

    expense = loadedExpense;

    setError("");
    return true;
}
