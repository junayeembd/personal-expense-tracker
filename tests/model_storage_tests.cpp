#include "Budget.h"
#include "BudgetStorage.h"
#include "Expense.h"
#include "ExpenseStorage.h"
#include "Income.h"
#include "IncomeStorage.h"
#include "RecurringRule.h"
#include "RecurringStorage.h"

#include <QCoreApplication>
#include <QJsonObject>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>

#include <cmath>
#include <iostream>

namespace {
int failures = 0;

void check(bool condition, const char* description)
{
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
    } else {
        std::cerr << "[FAIL] " << description << '\n';
        ++failures;
    }
}
}

int main(int argc, char* argv[])
{
    QCoreApplication app(argc, argv);

    // Budget model validation and JSON round-trip.
    Budget budget("budget-1", "2026-11", "Food", 12000.0);
    QString error;
    check(budget.isValid(&error), "valid monthly budget passes validation");
    const QJsonObject budgetJson = budget.toJson();
    Budget parsedBudget;
    check(Budget::fromJson(budgetJson, parsedBudget, &error),
          "budget JSON can be parsed");
    check(parsedBudget.getMonth() == "2026-11" &&
          parsedBudget.getCategory() == "Food" &&
          std::abs(parsedBudget.getLimitAmount() - 12000.0) < 0.001,
          "budget JSON round-trip preserves values");

    Budget badBudget("budget-2", "2026-13", "Food", 0.0);
    check(!badBudget.isValid(&error), "invalid budget month/amount is rejected");

    QTemporaryDir temp;
    check(temp.isValid(), "temporary data directory is available");
    if (!temp.isValid()) return 1;

    // Budget storage and duplicate protection.
    BudgetStorage budgetStorage(temp.filePath("budgets.json"));
    QList<Budget> budgets{budget};
    check(budgetStorage.save(budgets, &error), "budget storage saves JSON");
    QList<Budget> loadedBudgets;
    check(budgetStorage.load(loadedBudgets, &error), "budget storage loads JSON");
    check(loadedBudgets.size() == 1 && loadedBudgets.first().getId() == "budget-1",
          "budget storage preserves the saved record");
    budgets.append(Budget("budget-3", "2026-11", "food", 9000.0));
    check(!budgetStorage.save(budgets, &error),
          "duplicate category/month budgets are rejected");

    // Expense legacy compatibility (older records without an ID).
    QJsonObject legacyExpense;
    legacyExpense.insert("date", "2026-11-03");
    legacyExpense.insert("category", "Transport");
    legacyExpense.insert("amount", 150.0);
    legacyExpense.insert("note", "Bus");
    Expense migratedExpense;
    check(Expense::fromJson(legacyExpense, migratedExpense, &error),
          "legacy expense without an ID is accepted");
    check(!migratedExpense.getId().isEmpty(),
          "legacy expense receives a generated ID");

    ExpenseStorage expenseStorage(temp.filePath("expenses.json"));
    QList<Expense> expenses{migratedExpense};
    check(expenseStorage.save(expenses, &error), "expense storage saves JSON");
    QList<Expense> loadedExpenses;
    check(expenseStorage.load(loadedExpenses, &error) && loadedExpenses.size() == 1,
          "expense storage loads saved records");

    // Income model and storage.
    Income income("income-1", QDate(2026, 11, 1), "Salary", 45000.0, "Monthly pay");
    check(income.isValid(&error), "valid income passes validation");
    IncomeStorage incomeStorage(temp.filePath("incomes.json"));
    QList<Income> incomes{income};
    check(incomeStorage.save(incomes, &error), "income storage saves JSON");
    QList<Income> loadedIncomes;
    check(incomeStorage.load(loadedIncomes, &error) && loadedIncomes.size() == 1,
          "income storage loads saved records");

    // Recurring rule validation, JSON round-trip, and persistence.
    RecurringRule rule("rule-1", "Expense", "Rent", 15000.0,
                       "Monthly", QDate(2026, 11, 5), "Apartment rent", true);
    check(rule.isValid(&error), "valid recurring rule passes validation");
    RecurringRule parsedRule;
    check(RecurringRule::fromJson(rule.toJson(), parsedRule, &error),
          "recurring rule JSON can be parsed");
    check(parsedRule.getFrequency() == "Monthly" &&
          parsedRule.getNextDate() == QDate(2026, 11, 5) &&
          parsedRule.isActive(), "recurring rule JSON round-trip preserves values");
    check(rule.transactionIdForDate(QDate(2026, 11, 5)) ==
          rule.transactionIdForDate(QDate(2026, 11, 5)),
          "recurring occurrence IDs are deterministic");
    RecurringRule monthEndRule("rule-month-end", "Expense", "Rent", 1000.0,
                               "Monthly", QDate(2027, 1, 31), "", true);
    check(monthEndRule.nextOccurrenceAfter(QDate(2027, 1, 31)) == QDate(2027, 2, 28),
          "monthly schedule keeps end-of-month when February is shorter");
    check(monthEndRule.nextOccurrenceAfter(QDate(2027, 2, 28)) == QDate(2027, 3, 31),
          "monthly end-of-month schedule returns to the end of March");
    RecurringRule restoredMonthEndRule;
    check(RecurringRule::fromJson(monthEndRule.toJson(), restoredMonthEndRule, &error) &&
          restoredMonthEndRule.nextOccurrenceAfter(QDate(2027, 2, 28)) == QDate(2027, 3, 31),
          "monthly anchor day survives JSON storage");
    RecurringRule dayThirtyRule("rule-day-30", "Expense", "Phone", 500.0,
                                "Monthly", QDate(2027, 1, 30), "", true);
    check(dayThirtyRule.nextOccurrenceAfter(QDate(2027, 1, 30)) == QDate(2027, 2, 28),
          "monthly day 30 adjusts for February");
    check(dayThirtyRule.nextOccurrenceAfter(QDate(2027, 2, 28)) == QDate(2027, 3, 30),
          "monthly day 30 restores the original day after February");
    RecurringRule weeklyRule("rule-weekly", "Expense", "Travel", 100.0,
                             "Weekly", QDate(2026, 11, 3), "", true);
    check(weeklyRule.nextOccurrenceAfter(QDate(2026, 11, 3)) == QDate(2026, 11, 10),
          "weekly schedule advances by seven days");
    RecurringRule yearlyRule("rule-yearly", "Expense", "Insurance", 5000.0,
                             "Yearly", QDate(2024, 2, 29), "", true);
    check(yearlyRule.nextOccurrenceAfter(QDate(2024, 2, 29)) == QDate(2025, 2, 28),
          "yearly schedule safely handles leap day");
    RecurringRule badRule("rule-2", "Expense", "Rent", 15000.0,
                          "Daily", QDate(2026, 11, 5), "", true);
    check(!badRule.isValid(&error), "unsupported recurring frequency is rejected");
    QJsonObject malformedAnchor = monthEndRule.toJson();
    malformedAnchor.insert("monthlyDay", "31");
    RecurringRule rejectedMalformedAnchor;
    check(!RecurringRule::fromJson(malformedAnchor, rejectedMalformedAnchor, &error),
          "malformed monthly anchor type is rejected");

    RecurringStorage recurringStorage(temp.filePath("recurring.json"));
    QList<RecurringRule> rules{rule};
    check(recurringStorage.save(rules, &error), "recurring storage saves JSON");
    QList<RecurringRule> loadedRules;
    check(recurringStorage.load(loadedRules, &error) && loadedRules.size() == 1,
          "recurring storage loads saved rules");
    check(loadedRules.first().getId() == "rule-1",
          "recurring storage preserves rule identity");
    rules.append(RecurringRule("rule-1", "Income", "Salary", 1000.0,
                               "Monthly", QDate(2026, 12, 1), "duplicate", true));
    check(!recurringStorage.save(rules, &error),
          "recurring storage rejects duplicate rule IDs");
    const QString malformedRecurringPath = temp.filePath("malformed-recurring.json");
    QFile malformedRecurringFile(malformedRecurringPath);
    check(malformedRecurringFile.open(QIODevice::WriteOnly),
          "malformed recurring test file can be created");
    if (malformedRecurringFile.isOpen()) {
        malformedRecurringFile.write("{not valid json");
        malformedRecurringFile.close();
    }
    RecurringStorage malformedRecurringStorage(malformedRecurringPath);
    QList<RecurringRule> unchangedRules{rule};
    check(!malformedRecurringStorage.load(unchangedRules, &error) &&
          unchangedRules.size() == 1 && unchangedRules.first().getId() == "rule-1",
          "malformed recurring JSON is rejected without replacing in-memory data");

    std::cout << "\n" << (failures == 0 ? "All tests passed." : "Some tests failed.")
              << " Failures: " << failures << '\n';
    return failures == 0 ? 0 : 1;
}
