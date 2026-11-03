#include "RecurringStorage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <QSet>

RecurringStorage::RecurringStorage(const QString& filePath) : m_filePath(filePath) {}
QString RecurringStorage::getFilePath() const { return m_filePath; }

bool RecurringStorage::load(QList<RecurringRule>& rules, QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    QFile file(m_filePath);
    if (!file.exists()) {
        rules.clear();
        if (errorMessage) errorMessage->clear();
        return true;
    }
    if (!file.open(QIODevice::ReadOnly))
        return fail("Could not open recurring data: " + file.errorString());

    const QByteArray bytes = file.readAll();
    file.close();
    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError)
        return fail("Recurring JSON error: " + parseError.errorString());
    if (!document.isObject())
        return fail("Recurring JSON root must be an object.");

    const QJsonValue value = document.object().value("rules");
    if (!value.isArray())
        return fail("Recurring JSON must contain a 'rules' array.");

    QList<RecurringRule> loaded;
    QSet<QString> ids;
    const QJsonArray records = value.toArray();
    for (qsizetype i = 0; i < records.size(); ++i) {
        if (!records.at(i).isObject())
            return fail(QString("Invalid recurring rule at row %1.").arg(i + 1));
        RecurringRule rule;
        QString validationError;
        if (!RecurringRule::fromJson(records.at(i).toObject(), rule, &validationError))
            return fail(QString("Recurring rule %1: %2").arg(i + 1).arg(validationError));
        if (ids.contains(rule.getId()))
            return fail(QString("Duplicate recurring rule ID at row %1.").arg(i + 1));
        ids.insert(rule.getId());
        loaded.append(rule);
    }

    rules = loaded;
    if (errorMessage) errorMessage->clear();
    return true;
}

bool RecurringStorage::save(const QList<RecurringRule>& rules, QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    const QFileInfo info(m_filePath);
    if (!QDir().mkpath(info.absolutePath()))
        return fail("Could not create recurring data directory.");

    QJsonArray records;
    QSet<QString> ids;
    for (const RecurringRule& rule : rules) {
        QString validationError;
        if (!rule.isValid(&validationError)) return fail(validationError);
        if (ids.contains(rule.getId()))
            return fail("Duplicate recurring rule ID: " + rule.getId());
        ids.insert(rule.getId());
        records.append(rule.toJson());
    }

    QJsonObject root;
    root.insert("rules", records);
    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
    QSaveFile file(m_filePath);
    if (!file.open(QIODevice::WriteOnly))
        return fail("Could not save recurring data: " + file.errorString());
    if (file.write(json) != json.size()) {
        file.cancelWriting();
        return fail("Could not write complete recurring data.");
    }
    if (!file.commit())
        return fail("Could not commit recurring data: " + file.errorString());

    if (errorMessage) errorMessage->clear();
    return true;
}
