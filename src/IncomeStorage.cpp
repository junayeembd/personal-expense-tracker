#include "IncomeStorage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>

IncomeStorage::IncomeStorage(const QString& filePath)
    : m_filePath(filePath)
{
}

QString IncomeStorage::getFilePath() const { return m_filePath; }

bool IncomeStorage::load(QList<Income>& incomes, QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    QFile file(m_filePath);
    if (!file.exists()) {
        incomes.clear();
        if (errorMessage) errorMessage->clear();
        return true;
    }

    if (!file.open(QIODevice::ReadOnly))
        return fail("Could not open income data: " + file.errorString());

    const QByteArray bytes = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return fail("Income JSON error: " + parseError.errorString());
    if (!document.isObject())
        return fail("Income JSON root must be an object.");

    const QJsonValue recordsValue = document.object().value("incomes");
    if (!recordsValue.isArray())
        return fail("Income JSON must contain an 'incomes' array.");

    QList<Income> loaded;
    const QJsonArray records = recordsValue.toArray();
    for (qsizetype i = 0; i < records.size(); ++i) {
        if (!records.at(i).isObject())
            return fail(QString("Invalid income record at row %1.").arg(i + 1));

        Income income;
        QString validationError;
        if (!Income::fromJson(records.at(i).toObject(), income, &validationError))
            return fail(QString("Income record %1: %2").arg(i + 1).arg(validationError));
        loaded.append(income);
    }

    incomes = loaded;
    if (errorMessage) errorMessage->clear();
    return true;
}

bool IncomeStorage::save(const QList<Income>& incomes, QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    const QFileInfo info(m_filePath);
    if (!QDir().mkpath(info.absolutePath()))
        return fail("Could not create income data directory.");

    QJsonArray records;
    for (const Income& income : incomes) {
        QString validationError;
        if (!income.isValid(&validationError))
            return fail(validationError);
        records.append(income.toJson());
    }

    QJsonObject root;
    root.insert("incomes", records);
    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);

    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
        return fail("Could not save income data: " + file.errorString());
    if (file.write(json) != json.size()) {
        file.cancelWriting();
        return fail("Could not write the complete income data file.");
    }
    if (!file.commit())
        return fail("Could not commit income data: " + file.errorString());

    if (errorMessage) errorMessage->clear();
    return true;
}
