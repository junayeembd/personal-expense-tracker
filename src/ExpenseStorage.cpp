
#include "ExpenseStorage.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QSet>

// Constructor
ExpenseStorage::ExpenseStorage(const QString& filePath)
    : m_filePath(filePath)
{
}

// Get file path
QString ExpenseStorage::getFilePath() const
{
    return m_filePath;
}

// Load expenses from JSON
bool ExpenseStorage::load(
    QList<Expense>& expenses,
    QString* errorMessage
) const
{
    auto setError = [&](const QString& message) {
        if (errorMessage)
            *errorMessage = message;
    };

    QFile file(m_filePath);

    // File does not exist yet
    if (!file.exists()) {
        expenses.clear();
        setError("");
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        setError("Could not open file: " + file.errorString());
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;

    QJsonDocument document =
        QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        setError(
            "JSON Error: " + parseError.errorString()
        );
        return false;
    }

    if (!document.isObject()) {
        setError("Invalid JSON root object.");
        return false;
    }

    const QJsonValue recordsValue = document.object().value("expenses");
    if (!recordsValue.isArray()) {
        setError("Expense JSON must contain an 'expenses' array.");
        return false;
    }
    const QJsonArray array = recordsValue.toArray();

    // Read and validate each expense
    QList<Expense> loadedExpenses;
    QSet<QString> ids;

    for (int i = 0; i < array.size(); ++i) {

        if (!array.at(i).isObject()) {
            setError(
                QString("Invalid expense at index %1").arg(i)
            );
            return false;
        }

        Expense expense;

        QString validationError;

        if (!Expense::fromJson(
                array.at(i).toObject(),
                expense,
                &validationError
            )) {

            setError(
                QString("Expense %1: %2")
                    .arg(i + 1)
                    .arg(validationError)
            );

            return false;
        }

        if (ids.contains(expense.getId())) {
            setError(QString("Duplicate expense ID at record %1.").arg(i + 1));
            return false;
        }
        ids.insert(expense.getId());
        loadedExpenses.append(expense);
    }

    expenses = loadedExpenses;

    setError("");
    return true;
}

// Save expenses to JSON
bool ExpenseStorage::save(
    const QList<Expense>& expenses,
    QString* errorMessage
) const
{
    auto setError = [&](const QString& message) {
        if (errorMessage)
            *errorMessage = message;
    };

    QFileInfo fileInfo(m_filePath);

    if (!QDir().mkpath(fileInfo.absolutePath())) {
        setError("Could not create data directory.");
        return false;
    }

    QJsonArray array;
    QSet<QString> ids;

    for (const Expense& expense : expenses) {

        QString validationError;

        if (!expense.isValid(&validationError)) {
            setError(validationError);
            return false;
        }

        if (ids.contains(expense.getId())) {
            setError("Duplicate expense ID: " + expense.getId());
            return false;
        }
        ids.insert(expense.getId());
        array.append(expense.toJson());
    }

    QJsonObject root;

    root.insert("expenses", array);

    QJsonDocument document(root);

    QByteArray jsonData =
        document.toJson(QJsonDocument::Indented);

    // Atomic file saving
    QSaveFile file(m_filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        setError("Could not save file: " + file.errorString());
        return false;
    }

    if (file.write(jsonData) != jsonData.size()) {
        setError("Failed to write complete JSON data.");
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        setError("Could not commit JSON file: " + file.errorString());
        return false;
    }

    setError("");
    return true;
}
