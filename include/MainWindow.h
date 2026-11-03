#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>

#include "Expense.h"
#include "ExpenseStorage.h"
#include "Income.h"
#include "IncomeStorage.h"
#include "Budget.h"
#include "BudgetStorage.h"
#include "RecurringRule.h"
#include "RecurringStorage.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QProgressBar;
class QDateEdit;
class QPushButton;
class QTableWidget;
class QStackedWidget;
class QPropertyAnimation;
class QGraphicsOpacityEffect;
class QWidget;
class MonthlyTrendChart;
class CategoryDonutChart;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(const QString& dataDirectory,
                        QWidget* parent = nullptr);

private:
    QList<Expense> expenses;
    QList<Income> incomes;
    QList<Budget> budgets;
    QList<RecurringRule> recurringRules;
    ExpenseStorage expenseStorage;
    IncomeStorage incomeStorage;
    BudgetStorage budgetStorage;
    RecurringStorage recurringStorage;
    bool expenseStorageReady = false;
    bool incomeStorageReady = false;
    bool budgetStorageReady = false;
    bool recurringStorageReady = false;

    QStackedWidget* pageStack = nullptr;
    QGraphicsOpacityEffect* pageOpacityEffect = nullptr;
    QPropertyAnimation* pageFadeAnimation = nullptr;
    QPushButton* dashboardNavButton = nullptr;
    QPushButton* transactionsNavButton = nullptr;
    QPushButton* budgetsNavButton = nullptr;
    QPushButton* recurringNavButton = nullptr;

    QLabel* totalIncomeValue = nullptr;
    QLabel* totalExpenseValue = nullptr;
    QLabel* balanceValue = nullptr;
    QLabel* monthExpenseValue = nullptr;
    MonthlyTrendChart* trendChart = nullptr;
    CategoryDonutChart* categoryChart = nullptr;
    QLabel* budgetSummaryLabel = nullptr;
    QProgressBar* dashboardBudgetProgress = nullptr;

    QLineEdit* searchInput = nullptr;
    QComboBox* typeFilter = nullptr;
    QComboBox* categoryFilter = nullptr;
    QCheckBox* dateFilterCheck = nullptr;
    QDateEdit* fromDate = nullptr;
    QDateEdit* toDate = nullptr;
    QComboBox* sortFilter = nullptr;
    QTableWidget* transactionTable = nullptr;
    QLabel* visibleCountLabel = nullptr;

    QPushButton* addExpenseButton = nullptr;
    QPushButton* addIncomeButton = nullptr;
    QPushButton* editButton = nullptr;
    QPushButton* deleteButton = nullptr;
    QPushButton* exportButton = nullptr;

    QDateEdit* budgetMonthFilter = nullptr;
    QTableWidget* budgetTable = nullptr;
    QLabel* budgetCountLabel = nullptr;
    QPushButton* addBudgetButton = nullptr;
    QPushButton* editBudgetButton = nullptr;
    QPushButton* deleteBudgetButton = nullptr;
    QPushButton* backupButton = nullptr;
    QPushButton* restoreButton = nullptr;

    QTableWidget* recurringTable = nullptr;
    QLabel* recurringCountLabel = nullptr;
    QPushButton* addRecurringButton = nullptr;
    QPushButton* processRecurringButton = nullptr;
    QPushButton* editRecurringButton = nullptr;
    QPushButton* toggleRecurringButton = nullptr;
    QPushButton* deleteRecurringButton = nullptr;

    void buildUI();
    QWidget* buildDashboardPage();
    QWidget* buildTransactionsPage();
    QWidget* buildBudgetsPage();
    QWidget* buildRecurringPage();
    void navigateTo(int page);

    void refreshAll();
    void refreshDashboard();
    void refreshTransactions();
    void refreshBudgets();
    void refreshRecurring();
    void processDueRecurring(bool showSummary);
    double spentForBudget(const Budget& budget) const;
    bool saveExpenseData();
    bool saveIncomeData();
    bool saveBudgetData();
    bool saveRecurringData();

    void addRecord(bool isIncome);
    void editSelectedRecord();
    void deleteSelectedRecord();
    void exportCsv();
    void addBudget();
    void editSelectedBudget();
    void deleteSelectedBudget();
    void addRecurringRule();
    void editSelectedRecurringRule();
    void toggleSelectedRecurringRule();
    void deleteSelectedRecurringRule();
    void backupData();
    void restoreData();
    bool writeBackupFile(const QString& path, QString* errorMessage = nullptr) const;

    QString selectedRecordId() const;
    bool selectedRecordIsIncome() const;
};

#endif
