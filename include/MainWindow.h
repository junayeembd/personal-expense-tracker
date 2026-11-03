#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>

#include "Expense.h"
#include "ExpenseStorage.h"
#include "Income.h"
#include "IncomeStorage.h"

class QLabel;
class QLineEdit;
class QComboBox;
class QCheckBox;
class QDateEdit;
class QPushButton;
class QTableWidget;
class QStackedWidget;
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
    ExpenseStorage expenseStorage;
    IncomeStorage incomeStorage;
    bool expenseStorageReady = false;
    bool incomeStorageReady = false;

    QStackedWidget* pageStack = nullptr;
    QPushButton* dashboardNavButton = nullptr;
    QPushButton* transactionsNavButton = nullptr;

    QLabel* totalIncomeValue = nullptr;
    QLabel* totalExpenseValue = nullptr;
    QLabel* balanceValue = nullptr;
    QLabel* monthExpenseValue = nullptr;
    MonthlyTrendChart* trendChart = nullptr;
    CategoryDonutChart* categoryChart = nullptr;

    QLineEdit* searchInput = nullptr;
    QComboBox* typeFilter = nullptr;
    QComboBox* categoryFilter = nullptr;
    QCheckBox* dateFilterCheck = nullptr;
    QDateEdit* fromDate = nullptr;
    QDateEdit* toDate = nullptr;
    QTableWidget* transactionTable = nullptr;
    QLabel* visibleCountLabel = nullptr;

    QPushButton* addExpenseButton = nullptr;
    QPushButton* addIncomeButton = nullptr;
    QPushButton* editButton = nullptr;
    QPushButton* deleteButton = nullptr;
    QPushButton* exportButton = nullptr;

    void buildUI();
    QWidget* buildDashboardPage();
    QWidget* buildTransactionsPage();
    void navigateTo(int page);

    void refreshAll();
    void refreshDashboard();
    void refreshTransactions();
    bool saveExpenseData();
    bool saveIncomeData();

    void addRecord(bool isIncome);
    void editSelectedRecord();
    void deleteSelectedRecord();
    void exportCsv();

    QString selectedRecordId() const;
    bool selectedRecordIsIncome() const;
};

#endif
