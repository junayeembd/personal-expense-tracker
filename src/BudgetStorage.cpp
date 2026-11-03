#include "BudgetStorage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <QSet>

BudgetStorage::BudgetStorage(const QString& filePath)
    : m_filePath(filePath)
{
}

QString BudgetStorage::getFilePath() const
{
    return m_filePath;
}

bool BudgetStorage::load(QList<Budget>& budgets, QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    QFile file(m_filePath);
    if (!file.exists()) {
        budgets.clear();
        if (errorMessage) errorMessage->clear();
        return true;
    }
    if (!file.open(QIODevice::ReadOnly))
        return fail("Could not open budget data: " + file.errorString());

    const QByteArray bytes = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return fail("Budget JSON error: " + parseError.errorString());
    if (!document.isObject())
        return fail("Budget JSON root must be an object.");

    const QJsonValue arrayValue = document.object().value("budgets");
    if (!arrayValue.isArray())
        return fail("Budget JSON must contain a 'budgets' array.");

    QList<Budget> loaded;
    QSet<QString> keys;
    QSet<QString> ids;
    const QJsonArray records = arrayValue.toArray();

    for (qsizetype i = 0; i < records.size(); ++i) {
        if (!records.at(i).isObject())
            return fail(QString("Invalid budget record at row %1.").arg(i + 1));

        Budget budget;
        QString validationError;
        if (!Budget::fromJson(records.at(i).toObject(), budget, &validationError))
            return fail(QString("Budget record %1: %2").arg(i + 1).arg(validationError));

        if (ids.contains(budget.getId()))
            return fail(QString("Duplicate budget ID at row %1.").arg(i + 1));
        ids.insert(budget.getId());

        const QString key = budget.getMonth() + QChar(0x1f) +
                            budget.getCategory().toCaseFolded();
        if (keys.contains(key)) {
            return fail(QString("Duplicate budget for %1 in %2.")
                            .arg(budget.getCategory(), budget.getMonth()));
        }
        keys.insert(key);
        loaded.append(budget);
    }

    budgets = loaded;
    if (errorMessage) errorMessage->clear();
    return true;
}

bool BudgetStorage::save(const QList<Budget>& budgets, QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    const QFileInfo info(m_filePath);
    if (!QDir().mkpath(info.absolutePath()))
        return fail("Could not create budget data directory.");

    QJsonArray records;
    QSet<QString> keys;
    QSet<QString> ids;

    for (const Budget& budget : budgets) {
        QString validationError;
        if (!budget.isValid(&validationError))
            return fail(validationError);

        if (ids.contains(budget.getId()))
            return fail("Duplicate budget ID: " + budget.getId());
        ids.insert(budget.getId());

        const QString key = budget.getMonth() + QChar(0x1f) +
                            budget.getCategory().toCaseFolded();
        if (keys.contains(key)) {
            return fail(QString("A budget already exists for %1 in %2.")
                            .arg(budget.getCategory(), budget.getMonth()));
        }
        keys.insert(key);
        records.append(budget.toJson());
    }

    QJsonObject root;
    root.insert("budgets", records);
    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);

    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
        return fail("Could not save budget data: " + file.errorString());
    if (file.write(json) != json.size()) {
        file.cancelWriting();
        return fail("Could not write the complete budget data file.");
    }
    if (!file.commit())
        return fail("Could not commit budget data: " + file.errorString());

    if (errorMessage) errorMessage->clear();
    return true;
}
