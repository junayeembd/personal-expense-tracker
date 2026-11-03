#include "MainWindow.h"

#include <QApplication>
#include <QByteArray>
#include <QColor>
#include <QDir>
#include <QPaintEvent>
#include <QSizePolicy>
#include <QStringList>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QCheckBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QSaveFile>
#include <QPainter>
#include <QDate>
#include <QLocale>
#include <QUuid>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QEasingCurve>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QVector>
#include <QPair>
#include <QPoint>
#include <QRectF>
#include <QMap>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QStandardPaths>
#include <QDateTime>
#include <QSignalBlocker>
#include <QSet>

#include <algorithm>
#include <cmath>
#include <iterator>

namespace {

QString moneyText(double amount)
{
    return QString("BDT %1").arg(amount, 0, 'f', 2);
}

QString csvEscape(QString value)
{
    value.replace('"', "\"\"");
    // Reduce the risk of spreadsheet formula injection in text cells.
    if (!value.isEmpty() && QString("=+-@\t\r").contains(value.at(0)))
        value.prepend(QChar(39));
    return QString("\"") + value + QString("\"");
}

struct TableRecord {
    QString id;
    QString type;
    QDate date;
    QString category;
    double amount = 0.0;
    QString note;
};

} // namespace

// ------------------------------------------------------------
// Lightweight charts drawn with QPainter (no Qt Charts package)
// ------------------------------------------------------------

class MonthlyTrendChart : public QWidget
{
public:
    explicit MonthlyTrendChart(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(285);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setData(const QStringList& labels,
                 const QVector<double>& income,
                 const QVector<double>& expense)
    {
        m_labels = labels;
        m_income = income;
        m_expense = expense;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), Qt::white);

        p.setPen(QColor("#0f172a"));
        QFont heading = p.font();
        heading.setBold(true);
        heading.setPointSize(11);
        p.setFont(heading);
        p.drawText(QRect(18, 12, width() - 36, 22), Qt::AlignLeft,
                   "Income vs Expense — last 6 months");

        p.setFont(QFont("Segoe UI", 8));
        p.setPen(QColor("#64748b"));
        p.setBrush(QColor("#16a34a"));
        p.drawRoundedRect(QRect(20, 42, 10, 10), 2, 2);
        p.drawText(35, 51, "Income");
        p.setBrush(QColor("#ef4444"));
        p.drawRoundedRect(QRect(100, 42, 10, 10), 2, 2);
        p.drawText(115, 51, "Expense");

        if (m_labels.isEmpty()) {
            p.drawText(rect(), Qt::AlignCenter, "No financial data yet");
            return;
        }

        const QRectF plot(48, 68, qMax(100, width() - 66), qMax(100, height() - 112));
        double maxValue = 0.0;
        for (double value : m_income) maxValue = qMax(maxValue, value);
        for (double value : m_expense) maxValue = qMax(maxValue, value);
        if (maxValue <= 0.0) maxValue = 1.0;
        maxValue *= 1.15;

        p.setFont(QFont("Segoe UI", 8));
        for (int i = 0; i <= 4; ++i) {
            const double fraction = static_cast<double>(i) / 4.0;
            const qreal y = plot.bottom() - fraction * plot.height();
            p.setPen(QPen(QColor("#e2e8f0"), 1, Qt::DashLine));
            p.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
            p.setPen(QColor("#64748b"));
            p.drawText(QRectF(1, y - 8, 42, 16), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(maxValue * fraction, 'f', 0));
        }

        const int count = m_labels.size();
        const qreal groupWidth = plot.width() / qMax(1, count);
        const qreal barWidth = qMin<qreal>(22.0, groupWidth * 0.26);

        for (int i = 0; i < count; ++i) {
            const double inValue = i < m_income.size() ? m_income.at(i) : 0.0;
            const double outValue = i < m_expense.size() ? m_expense.at(i) : 0.0;
            const qreal center = plot.left() + groupWidth * (i + 0.5);
            const qreal inHeight = (inValue / maxValue) * plot.height();
            const qreal outHeight = (outValue / maxValue) * plot.height();

            p.setPen(Qt::NoPen);
            p.setBrush(QColor("#16a34a"));
            p.drawRoundedRect(QRectF(center - barWidth - 2,
                                     plot.bottom() - inHeight,
                                     barWidth, inHeight), 3, 3);
            p.setBrush(QColor("#ef4444"));
            p.drawRoundedRect(QRectF(center + 2,
                                     plot.bottom() - outHeight,
                                     barWidth, outHeight), 3, 3);

            p.setPen(QColor("#64748b"));
            p.drawText(QRectF(center - groupWidth / 2, plot.bottom() + 8,
                              groupWidth, 20), Qt::AlignHCenter | Qt::AlignTop,
                       m_labels.at(i));
        }
    }

private:
    QStringList m_labels;
    QVector<double> m_income;
    QVector<double> m_expense;
};

class CategoryDonutChart : public QWidget
{
public:
    explicit CategoryDonutChart(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        setMinimumHeight(285);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    }

    void setData(const QVector<QPair<QString, double>>& values)
    {
        m_values = values;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), Qt::white);

        p.setPen(QColor("#0f172a"));
        QFont heading = p.font();
        heading.setBold(true);
        heading.setPointSize(11);
        p.setFont(heading);
        p.drawText(QRect(18, 12, width() - 36, 22), Qt::AlignLeft,
                   "Expense breakdown — this month");

        double total = 0.0;
        for (const auto& item : m_values) total += item.second;
        if (total <= 0.0) {
            p.setPen(QColor("#64748b"));
            p.setFont(QFont("Segoe UI", 9));
            p.drawText(rect().adjusted(10, 35, -10, -10), Qt::AlignCenter,
                       "Add expenses to see category breakdown");
            return;
        }

        const QVector<QColor> colors = {
            QColor("#2563eb"), QColor("#16a34a"), QColor("#f59e0b"),
            QColor("#ef4444"), QColor("#8b5cf6"), QColor("#06b6d4"),
            QColor("#ec4899"), QColor("#64748b"), QColor("#84cc16")
        };

        const int diameter = qMin(190, qMax(130, height() - 90));
        const QRect pieRect(22, 62, diameter, diameter);
        int startAngle = 90 * 16;

        for (int i = 0; i < m_values.size(); ++i) {
            const double value = m_values.at(i).second;
            const int span = qRound((value / total) * 360.0 * 16.0);
            p.setPen(Qt::NoPen);
            p.setBrush(colors.at(i % colors.size()));
            p.drawPie(pieRect, startAngle, -span);
            startAngle -= span;
        }

        // Draw the white center to create a donut.
        const int hole = diameter / 2;
        const QPoint center = pieRect.center();
        p.setBrush(Qt::white);
        p.setPen(Qt::NoPen);
        p.drawEllipse(QRectF(center.x() - hole / 2.0,
                             center.y() - hole / 2.0,
                             hole, hole));
        p.setPen(QColor("#0f172a"));
        QFont centerFont("Segoe UI", 9, QFont::Bold);
        p.setFont(centerFont);
        p.drawText(QRect(center.x() - hole / 2, center.y() - 15,
                         hole, 30), Qt::AlignCenter,
                   QString("BDT\n%1").arg(total, 0, 'f', 0));

        const int legendX = pieRect.right() + 24;
        const int availableWidth = qMax(110, width() - legendX - 12);
        int y = 76;
        p.setFont(QFont("Segoe UI", 8));
        for (int i = 0; i < m_values.size(); ++i) {
            if (y > height() - 22) break;
            p.setPen(Qt::NoPen);
            p.setBrush(colors.at(i % colors.size()));
            p.drawRoundedRect(QRect(legendX, y - 8, 10, 10), 2, 2);
            p.setPen(QColor("#334155"));
            const QString label = QString("%1  %2%")
                .arg(m_values.at(i).first)
                .arg((m_values.at(i).second / total) * 100.0, 0, 'f', 1);
            p.drawText(QRect(legendX + 17, y - 12, availableWidth - 17, 20),
                       Qt::AlignLeft | Qt::AlignVCenter, label);
            y += 25;
        }
    }

private:
    QVector<QPair<QString, double>> m_values;
};

// ------------------------------------------------------------
// Add / edit income and expense dialog
// ------------------------------------------------------------

class FinancialRecordDialog : public QDialog
{
public:
    explicit FinancialRecordDialog(bool isIncome, QWidget* parent = nullptr,
                                   const Expense* expense = nullptr,
                                   const Income* income = nullptr)
        : QDialog(parent), m_isIncome(isIncome)
    {
        setWindowTitle(isIncome
            ? (income ? "Edit Income" : "Add Income")
            : (expense ? "Edit Expense" : "Add Expense"));
        setMinimumWidth(410);

        auto* layout = new QVBoxLayout(this);
        auto* form = new QFormLayout();

        amountInput = new QLineEdit(this);
        amountInput->setPlaceholderText("Enter amount in BDT");
        auto* validator = new QDoubleValidator(0.01, 1000000000000.0,
                                                2, amountInput);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::c());
        amountInput->setValidator(validator);

        categoryInput = new QComboBox(this);
        if (m_isIncome) {
            categoryInput->addItems({"Salary", "Freelance", "Business",
                                     "Investment", "Gift", "Other"});
        } else {
            categoryInput->addItems({"Food", "Transport", "Education",
                                     "Shopping", "Bills", "Health",
                                     "Entertainment", "Other"});
        }
        categoryInput->setEditable(true);
        categoryInput->setInsertPolicy(QComboBox::NoInsert);

        dateInput = new QDateEdit(QDate::currentDate(), this);
        dateInput->setCalendarPopup(true);
        dateInput->setDisplayFormat("dd MMM yyyy");
        dateInput->setMinimumWidth(150);

        noteInput = new QLineEdit(this);
        noteInput->setPlaceholderText("Optional description");

        form->addRow("Amount (BDT):", amountInput);
        form->addRow(m_isIncome ? "Income source:" : "Category:", categoryInput);
        form->addRow("Date:", dateInput);
        form->addRow("Description:", noteInput);
        layout->addLayout(form);

        auto* buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
            bool ok = false;
            const double amount = QLocale::c().toDouble(amountInput->text(), &ok);
            if (!ok || !std::isfinite(amount) || amount <= 0.0) {
                QMessageBox::warning(this, "Invalid amount",
                                     "Enter a valid amount greater than zero.");
                return;
            }
            if (categoryInput->currentText().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Missing category",
                                     "Enter or select a category/source.");
                return;
            }
            accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        if (expense) {
            amountInput->setText(QString::number(expense->getAmount(), 'f', 2));
            categoryInput->setCurrentText(expense->getCategory());
            dateInput->setDate(expense->getDate());
            noteInput->setText(expense->getNote());
        } else if (income) {
            amountInput->setText(QString::number(income->getAmount(), 'f', 2));
            categoryInput->setCurrentText(income->getCategory());
            dateInput->setDate(income->getDate());
            noteInput->setText(income->getNote());
        }
    }

    double amount() const { return QLocale::c().toDouble(amountInput->text()); }
    QString category() const { return categoryInput->currentText(); }
    QDate date() const { return dateInput->date(); }
    QString note() const { return noteInput->text().trimmed(); }

private:
    bool m_isIncome;
    QLineEdit* amountInput = nullptr;
    QComboBox* categoryInput = nullptr;
    QDateEdit* dateInput = nullptr;
    QLineEdit* noteInput = nullptr;
 };

// ------------------------------------------------------------
// Monthly category budget dialog
// ------------------------------------------------------------

class BudgetDialog : public QDialog
{
public:
    explicit BudgetDialog(QWidget* parent = nullptr,
                          const Budget* existing = nullptr,
                          const QStringList& knownCategories = {})
        : QDialog(parent)
    {
        setWindowTitle(existing ? "Edit monthly budget" : "Create monthly budget");
        setMinimumWidth(430);

        auto* layout = new QVBoxLayout(this);
        auto* form = new QFormLayout();

        categoryInput = new QComboBox(this);
        categoryInput->setEditable(true);
        categoryInput->setInsertPolicy(QComboBox::NoInsert);
        categoryInput->addItems(knownCategories);
        categoryInput->setCurrentText("Food");

        monthInput = new QDateEdit(QDate::currentDate(), this);
        monthInput->setCalendarPopup(true);
        monthInput->setDisplayFormat("MMMM yyyy");
        monthInput->setMinimumDate(QDate(2000, 1, 1));
        monthInput->setMaximumDate(QDate(2100, 12, 31));
        monthInput->setMinimumWidth(190);

        amountInput = new QLineEdit(this);
        amountInput->setPlaceholderText("Enter monthly limit in BDT");
        auto* validator = new QDoubleValidator(0.01, 1000000000000.0, 2, amountInput);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::c());
        amountInput->setValidator(validator);

        form->addRow("Category:", categoryInput);
        form->addRow("Budget month:", monthInput);
        form->addRow("Monthly limit (BDT):", amountInput);
        layout->addLayout(form);

        auto* hint = new QLabel(
            "A budget applies to one category in one calendar month.\n"
            "You can create separate limits for different months.", this);
        hint->setWordWrap(true);
        hint->setStyleSheet("color: #64748b; font-size: 9pt;");
        layout->addWidget(hint);

        auto* buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttons);

        connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
            bool ok = false;
            const double amount = QLocale::c().toDouble(amountInput->text(), &ok);
            if (!ok || !std::isfinite(amount) || amount <= 0.0) {
                QMessageBox::warning(this, "Invalid budget",
                                     "Enter a monthly limit greater than zero.");
                return;
            }
            if (categoryInput->currentText().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Missing category",
                                     "Enter or select a category.");
                return;
            }
            accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        if (existing) {
            categoryInput->setCurrentText(existing->getCategory());
            monthInput->setDate(QDate::fromString(existing->getMonth() + "-01", Qt::ISODate));
            amountInput->setText(QString::number(existing->getLimitAmount(), 'f', 2));
        }
    }

    QString category() const { return categoryInput->currentText().trimmed(); }
    QString month() const { return monthInput->date().toString("yyyy-MM"); }
    double amount() const { return QLocale::c().toDouble(amountInput->text()); }

private:
    QComboBox* categoryInput = nullptr;
    QDateEdit* monthInput = nullptr;
    QLineEdit* amountInput = nullptr;
};

// ------------------------------------------------------------
// Recurring transaction rule dialog
// ------------------------------------------------------------

class RecurringDialog : public QDialog
{
public:
    explicit RecurringDialog(QWidget* parent = nullptr,
                              const RecurringRule* existing = nullptr,
                              const QStringList& knownCategories = {})
        : QDialog(parent)
    {
        setWindowTitle(existing ? "Edit recurring transaction" : "New recurring transaction");
        setMinimumWidth(450);

        auto* layout = new QVBoxLayout(this);
        auto* form = new QFormLayout();

        typeInput = new QComboBox(this);
        typeInput->addItems({"Expense", "Income"});

        categoryInput = new QComboBox(this);
        categoryInput->setEditable(true);
        categoryInput->setInsertPolicy(QComboBox::NoInsert);
        categoryInput->addItems(knownCategories);
        categoryInput->setCurrentText("Food");

        amountInput = new QLineEdit(this);
        amountInput->setPlaceholderText("Enter amount in BDT");
        auto* validator = new QDoubleValidator(0.01, 1000000000000.0, 2, amountInput);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::c());
        amountInput->setValidator(validator);

        frequencyInput = new QComboBox(this);
        frequencyInput->addItems({"Weekly", "Monthly", "Yearly"});

        nextDateInput = new QDateEdit(QDate::currentDate(), this);
        nextDateInput->setCalendarPopup(true);
        nextDateInput->setDisplayFormat("dd MMM yyyy");
        nextDateInput->setMinimumWidth(160);

        noteInput = new QLineEdit(this);
        noteInput->setPlaceholderText("Optional description");

        form->addRow("Transaction type:", typeInput);
        form->addRow("Category / source:", categoryInput);
        form->addRow("Amount (BDT):", amountInput);
        form->addRow("Frequency:", frequencyInput);
        form->addRow("Next due date:", nextDateInput);
        form->addRow("Description:", noteInput);
        layout->addLayout(form);

        auto* hint = new QLabel(
            "Active rules automatically create due transactions when the app starts.\n"
            "You can also process due rules manually from this page.", this);
        hint->setWordWrap(true);
        hint->setStyleSheet("color: #64748b; font-size: 9pt;");
        layout->addWidget(hint);

        auto* buttons = new QDialogButtonBox(
            QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttons);
        connect(buttons, &QDialogButtonBox::accepted, this, [this]() {
            bool ok = false;
            const double amount = QLocale::c().toDouble(amountInput->text(), &ok);
            if (!ok || !std::isfinite(amount) || amount <= 0.0) {
                QMessageBox::warning(this, "Invalid amount",
                                     "Enter an amount greater than zero.");
                return;
            }
            if (categoryInput->currentText().trimmed().isEmpty()) {
                QMessageBox::warning(this, "Missing category",
                                     "Enter or select a category/source.");
                return;
            }
            accept();
        });
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        if (existing) {
            typeInput->setCurrentText(existing->getType());
            categoryInput->setCurrentText(existing->getCategory());
            amountInput->setText(QString::number(existing->getAmount(), 'f', 2));
            frequencyInput->setCurrentText(existing->getFrequency());
            nextDateInput->setDate(existing->getNextDate());
            noteInput->setText(existing->getNote());
        }
    }

    QString type() const { return typeInput->currentText(); }
    QString category() const { return categoryInput->currentText().trimmed(); }
    double amount() const { return QLocale::c().toDouble(amountInput->text()); }
    QString frequency() const { return frequencyInput->currentText(); }
    QDate nextDate() const { return nextDateInput->date(); }
    QString note() const { return noteInput->text().trimmed(); }

private:
    QComboBox* typeInput = nullptr;
    QComboBox* categoryInput = nullptr;
    QLineEdit* amountInput = nullptr;
    QComboBox* frequencyInput = nullptr;
    QDateEdit* nextDateInput = nullptr;
    QLineEdit* noteInput = nullptr;
};

// ------------------------------------------------------------
// Main window
// ------------------------------------------------------------

MainWindow::MainWindow(const QString& dataDirectory, QWidget* parent)
    : QMainWindow(parent),
      expenseStorage(QDir(dataDirectory).filePath("expenses.json")),
      incomeStorage(QDir(dataDirectory).filePath("incomes.json")),
      budgetStorage(QDir(dataDirectory).filePath("budgets.json")),
      recurringStorage(QDir(dataDirectory).filePath("recurring.json"))
{
    setWindowTitle("Personal Finance Tracker | Version 3.0");
    resize(1320, 850);
    setMinimumSize(1050, 700);

    buildUI();

    QString expenseError;
    QString incomeError;
    QString budgetError;
    QString recurringError;
    expenseStorageReady = expenseStorage.load(expenses, &expenseError);
    incomeStorageReady = incomeStorage.load(incomes, &incomeError);
    budgetStorageReady = budgetStorage.load(budgets, &budgetError);
    recurringStorageReady = recurringStorage.load(recurringRules, &recurringError);

    if (!expenseStorageReady || !incomeStorageReady || !budgetStorageReady || !recurringStorageReady) {
        QString details;
        if (!expenseStorageReady) details += "Expense data: " + expenseError + "\n";
        if (!incomeStorageReady) details += "Income data: " + incomeError + "\n";
        if (!budgetStorageReady) details += "Budget data: " + budgetError + "\n";
        if (!recurringStorageReady) details += "Recurring data: " + recurringError;
        QMessageBox::critical(this, "Data loading error",
                              details + "\n\nThe affected data file could not be loaded. "
                              "It has been left untouched until you restore or repair it.");
    }

    addExpenseButton->setEnabled(expenseStorageReady);
    addIncomeButton->setEnabled(incomeStorageReady);
    addBudgetButton->setEnabled(budgetStorageReady);
    editBudgetButton->setEnabled(budgetStorageReady);
    deleteBudgetButton->setEnabled(budgetStorageReady);
    addRecurringButton->setEnabled(recurringStorageReady);
    processRecurringButton->setEnabled(recurringStorageReady);
    editRecurringButton->setEnabled(recurringStorageReady);
    toggleRecurringButton->setEnabled(recurringStorageReady);
    deleteRecurringButton->setEnabled(recurringStorageReady);
    if (expenseStorageReady && incomeStorageReady && recurringStorageReady)
        processDueRecurring(false);
    refreshAll();
}

void MainWindow::buildUI()
{
    auto* central = new QWidget(this);
    auto* root = new QHBoxLayout(central);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // Sidebar navigation
    auto* sidebar = new QWidget(central);
    sidebar->setObjectName("sidebar");
    sidebar->setFixedWidth(205);
    auto* sideLayout = new QVBoxLayout(sidebar);
    sideLayout->setContentsMargins(18, 25, 18, 20);
    sideLayout->setSpacing(12);

    auto* brand = new QLabel("PFT\nPERSONAL FINANCE", sidebar);
    brand->setObjectName("brandLabel");
    sideLayout->addWidget(brand);
    sideLayout->addSpacing(25);

    dashboardNavButton = new QPushButton("  Dashboard", sidebar);
    dashboardNavButton->setObjectName("navButton");
    dashboardNavButton->setCheckable(true);
    dashboardNavButton->setChecked(true);

    transactionsNavButton = new QPushButton("  Transactions", sidebar);
    transactionsNavButton->setObjectName("navButton");
    transactionsNavButton->setCheckable(true);

    budgetsNavButton = new QPushButton("  Budgets", sidebar);
    budgetsNavButton->setObjectName("navButton");
    budgetsNavButton->setCheckable(true);

    recurringNavButton = new QPushButton("  Recurring", sidebar);
    recurringNavButton->setObjectName("navButton");
    recurringNavButton->setCheckable(true);

    sideLayout->addWidget(dashboardNavButton);
    sideLayout->addWidget(transactionsNavButton);
    sideLayout->addWidget(budgetsNavButton);
    sideLayout->addWidget(recurringNavButton);
    sideLayout->addStretch();

    auto* localLabel = new QLabel("LOCAL DATA\nSaved on this device", sidebar);
    localLabel->setObjectName("sidebarFootnote");
    sideLayout->addWidget(localLabel);

    pageStack = new QStackedWidget(central);
    pageStack->addWidget(buildDashboardPage());
    pageStack->addWidget(buildTransactionsPage());
    pageStack->addWidget(buildBudgetsPage());
    pageStack->addWidget(buildRecurringPage());

    // A subtle fade makes page navigation feel smoother without slowing down the UI.
    pageOpacityEffect = new QGraphicsOpacityEffect(pageStack);
    pageOpacityEffect->setOpacity(1.0);
    pageStack->setGraphicsEffect(pageOpacityEffect);
    pageFadeAnimation = new QPropertyAnimation(pageOpacityEffect, "opacity", this);
    pageFadeAnimation->setDuration(170);
    pageFadeAnimation->setEasingCurve(QEasingCurve::OutCubic);

    root->addWidget(sidebar);
    root->addWidget(pageStack, 1);

    central->setStyleSheet(R"(
        QWidget { font-family: "Segoe UI"; font-size: 10pt; color: #1e293b; }
        QWidget#sidebar { background: #101b33; }
        QLabel#brandLabel { color: white; font-size: 17px; font-weight: bold; letter-spacing: 1px; }
        QLabel#sidebarFootnote { color: #a6b3c8; font-size: 9px; }
        QLabel#mutedText { color: #64748b; }
        QWidget#budgetCard { background: white; border: 1px solid #e2e8f0; border-radius: 12px; }
        QProgressBar { border: 1px solid #dbe3ef; border-radius: 6px; background: #f1f5f9;
            text-align: center; min-height: 17px; color: #1e293b; }
        QProgressBar::chunk { background: #16a34a; border-radius: 5px; }
        QProgressBar#warningProgress::chunk { background: #f59e0b; }
        QProgressBar#dangerProgress::chunk { background: #dc2626; }
        QPushButton#navButton { text-align: left; padding: 12px 13px; color: #cbd5e1;
            background: transparent; border: none; border-radius: 7px; font-weight: 600; }
        QPushButton#navButton:checked { background: #263b62; color: white; }
        QPushButton#navButton:hover { background: #1d2d4d; color: white; }
        QLabel#pageTitle { font-size: 25px; font-weight: bold; color: #0f172a; }
        QLabel#pageSubtitle { color: #64748b; }
        QWidget#statCard { background: white; border: 1px solid #e2e8f0; border-radius: 12px; }
        QLabel#cardHeading { color: #64748b; font-size: 10px; font-weight: bold; }
        QLabel#cardValue { color: #0f172a; font-size: 22px; font-weight: bold; }
        QWidget#chartCard { background: white; border: 1px solid #e2e8f0; border-radius: 12px; }
        QLineEdit, QComboBox, QDateEdit { padding: 9px; border: 1px solid #cbd5e1;
            border-radius: 7px; background: white; min-height: 20px; }
        QPushButton { padding: 10px 15px; border: none; border-radius: 7px;
            font-weight: bold; background: #e2e8f0; color: #1e293b; }
        QPushButton#primaryButton { background: #2563eb; color: white; }
        QPushButton#incomeButton { background: #dcfce7; color: #166534; }
        QPushButton#dangerButton { background: #fee2e2; color: #b91c1c; }
        QPushButton#secondaryButton { background: #e0e7ff; color: #3730a3; }
        QPushButton#primaryButton:hover { background: #1d4ed8; }
        QPushButton#incomeButton:hover { background: #bbf7d0; }
        QPushButton#dangerButton:hover { background: #fecaca; }
        QPushButton#secondaryButton:hover { background: #c7d2fe; }
        QLineEdit:focus, QComboBox:focus, QDateEdit:focus {
            border: 1px solid #2563eb;
        }
        QTableWidget { background: white; alternate-background-color: #f8fafc;
            gridline-color: #e2e8f0; border: 1px solid #e2e8f0; border-radius: 8px; }
        QTableWidget::item:selected { background: #dbeafe; color: #1e3a8a; }
        QHeaderView::section { background: #e8eef7; padding: 11px; border: none; font-weight: bold; }
    )");

    connect(dashboardNavButton, &QPushButton::clicked,
            this, [this]() { navigateTo(0); });
    connect(transactionsNavButton, &QPushButton::clicked,
            this, [this]() { navigateTo(1); });
    connect(budgetsNavButton, &QPushButton::clicked,
            this, [this]() { navigateTo(2); });
    connect(recurringNavButton, &QPushButton::clicked,
            this, [this]() { navigateTo(3); });

    setCentralWidget(central);
}

QWidget* MainWindow::buildDashboardPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 25, 28, 25);
    layout->setSpacing(17);

    auto* header = new QHBoxLayout();
    auto* titleBox = new QVBoxLayout();
    auto* title = new QLabel("Financial overview");
    title->setObjectName("pageTitle");
    auto* subtitle = new QLabel("Your income, spending and monthly trends at a glance");
    subtitle->setObjectName("pageSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    auto* goTransactions = new QPushButton("View transactions  →");
    goTransactions->setObjectName("primaryButton");
    backupButton = new QPushButton("Backup data");
    backupButton->setObjectName("secondaryButton");
    restoreButton = new QPushButton("Restore backup");
    restoreButton->setObjectName("secondaryButton");
    connect(goTransactions, &QPushButton::clicked,
            this, [this]() { navigateTo(1); });
    connect(backupButton, &QPushButton::clicked,
            this, [this]() { backupData(); });
    connect(restoreButton, &QPushButton::clicked,
            this, [this]() { restoreData(); });
    header->addLayout(titleBox);
    header->addStretch();
    header->addWidget(backupButton);
    header->addWidget(restoreButton);
    header->addWidget(goTransactions);
    layout->addLayout(header);

    auto* cards = new QGridLayout();
    cards->setHorizontalSpacing(14);
    cards->setVerticalSpacing(14);
    auto createCard = [](const QString& heading, QLabel*& value, const QString& objectName) {
        auto* card = new QWidget();
        card->setObjectName("statCard");
        auto* box = new QVBoxLayout(card);
        box->setContentsMargins(17, 15, 17, 15);
        auto* label = new QLabel(heading);
        label->setObjectName("cardHeading");
        value = new QLabel("BDT 0.00");
        value->setObjectName("cardValue");
        value->setProperty("metric", objectName);
        box->addWidget(label);
        box->addWidget(value);
        return card;
    };
    cards->addWidget(createCard("TOTAL INCOME", totalIncomeValue, "income"), 0, 0);
    cards->addWidget(createCard("TOTAL EXPENSE", totalExpenseValue, "expense"), 0, 1);
    cards->addWidget(createCard("CURRENT BALANCE", balanceValue, "balance"), 0, 2);
    cards->addWidget(createCard("THIS MONTH'S EXPENSE", monthExpenseValue, "month"), 0, 3);
    layout->addLayout(cards);

    auto* budgetCard = new QWidget();
    budgetCard->setObjectName("budgetCard");
    auto* budgetLayout = new QVBoxLayout(budgetCard);
    budgetLayout->setContentsMargins(16, 10, 16, 10);
    auto* budgetHeader = new QHBoxLayout();
    auto* budgetTitle = new QLabel("Monthly budget health");
    budgetTitle->setStyleSheet("font-weight: 700; color: #0f172a;");
    budgetSummaryLabel = new QLabel("No budgets set for this month");
    budgetSummaryLabel->setObjectName("mutedText");
    budgetHeader->addWidget(budgetTitle);
    budgetHeader->addStretch();
    budgetHeader->addWidget(budgetSummaryLabel);
    dashboardBudgetProgress = new QProgressBar();
    dashboardBudgetProgress->setRange(0, 100);
    dashboardBudgetProgress->setValue(0);
    dashboardBudgetProgress->setFormat("%p% of total monthly budget used");
    budgetLayout->addLayout(budgetHeader);
    budgetLayout->addWidget(dashboardBudgetProgress);
    layout->addWidget(budgetCard);

    auto* charts = new QGridLayout();
    charts->setHorizontalSpacing(14);
    charts->setVerticalSpacing(14);

    auto* trendCard = new QWidget();
    trendCard->setObjectName("chartCard");
    auto* trendLayout = new QVBoxLayout(trendCard);
    trendLayout->setContentsMargins(8, 8, 8, 8);
    trendChart = new MonthlyTrendChart(trendCard);
    trendLayout->addWidget(trendChart);

    auto* categoryCard = new QWidget();
    categoryCard->setObjectName("chartCard");
    auto* categoryLayout = new QVBoxLayout(categoryCard);
    categoryLayout->setContentsMargins(8, 8, 8, 8);
    categoryChart = new CategoryDonutChart(categoryCard);
    categoryLayout->addWidget(categoryChart);

    charts->addWidget(trendCard, 0, 0);
    charts->addWidget(categoryCard, 0, 1);
    charts->setColumnStretch(0, 1);
    charts->setColumnStretch(1, 1);
    layout->addLayout(charts, 1);

    auto* footnote = new QLabel(
        "Tip: Keep your income and expenses up to date to make the charts more useful.");
    footnote->setObjectName("pageSubtitle");
    layout->addWidget(footnote);
    return page;
}

QWidget* MainWindow::buildTransactionsPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 25, 28, 25);
    layout->setSpacing(14);

    auto* header = new QHBoxLayout();
    auto* titleBox = new QVBoxLayout();
    auto* title = new QLabel("Transactions");
    title->setObjectName("pageTitle");
    auto* subtitle = new QLabel("Add, edit, search and export your financial records");
    subtitle->setObjectName("pageSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    header->addLayout(titleBox);
    header->addStretch();

    addExpenseButton = new QPushButton("+ Expense");
    addExpenseButton->setObjectName("primaryButton");
    addIncomeButton = new QPushButton("+ Income");
    addIncomeButton->setObjectName("incomeButton");
    exportButton = new QPushButton("Export CSV");
    exportButton->setObjectName("secondaryButton");
    header->addWidget(addExpenseButton);
    header->addWidget(addIncomeButton);
    header->addWidget(exportButton);
    layout->addLayout(header);

    auto* filters = new QGridLayout();
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("Search category or description...");
    typeFilter = new QComboBox();
    typeFilter->addItems({"All types", "Expense", "Income"});
    categoryFilter = new QComboBox();
    categoryFilter->addItems({"All categories", "Food", "Transport", "Education",
                              "Shopping", "Bills", "Health", "Entertainment",
                              "Salary", "Freelance", "Business", "Investment",
                              "Gift", "Other"});
    sortFilter = new QComboBox();
    sortFilter->addItems({"Newest first", "Oldest first", "Amount: high to low", "Amount: low to high"});
    sortFilter->setMinimumWidth(155);
    sortFilter->setToolTip("Choose how transactions are ordered");
    dateFilterCheck = new QCheckBox("Date range");
    fromDate = new QDateEdit(QDate::currentDate().addYears(-5));
    toDate = new QDateEdit(QDate::currentDate());
    for (QDateEdit* dateEdit : {fromDate, toDate}) {
        dateEdit->setCalendarPopup(true);
        dateEdit->setDisplayFormat("dd MMM yyyy");
        dateEdit->setMinimumWidth(125);
        dateEdit->setEnabled(false);
    }

    filters->addWidget(searchInput, 0, 0);
    filters->addWidget(typeFilter, 0, 1);
    filters->addWidget(categoryFilter, 0, 2);
    filters->addWidget(sortFilter, 0, 3);
    filters->addWidget(dateFilterCheck, 1, 0);
    filters->addWidget(fromDate, 1, 1);
    filters->addWidget(toDate, 1, 2);
    auto* clearFiltersButton = new QPushButton("Clear filters");
    clearFiltersButton->setObjectName("secondaryButton");
    filters->addWidget(clearFiltersButton, 1, 3);
    filters->setColumnStretch(0, 3);
    filters->setColumnStretch(1, 1);
    filters->setColumnStretch(2, 2);
    filters->setColumnStretch(3, 1);
    layout->addLayout(filters);

    transactionTable = new QTableWidget();
    transactionTable->setColumnCount(6);
    transactionTable->setHorizontalHeaderLabels(
        {"Date", "Type", "Category", "Amount (BDT)", "Description", "Record ID"});
    transactionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    transactionTable->setColumnHidden(5, true);
    transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionTable->setSelectionMode(QAbstractItemView::SingleSelection);
    transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    transactionTable->setAlternatingRowColors(true);
    transactionTable->verticalHeader()->setVisible(false);
    transactionTable->setSortingEnabled(false);
    layout->addWidget(transactionTable, 1);

    auto* footer = new QHBoxLayout();
    visibleCountLabel = new QLabel("Showing 0 transactions");
    editButton = new QPushButton("Edit selected");
    editButton->setObjectName("secondaryButton");
    deleteButton = new QPushButton("Delete selected");
    deleteButton->setObjectName("dangerButton");
    footer->addWidget(visibleCountLabel);
    footer->addStretch();
    footer->addWidget(editButton);
    footer->addWidget(deleteButton);
    layout->addLayout(footer);

    connect(addExpenseButton, &QPushButton::clicked,
            this, [this]() { addRecord(false); });
    connect(addIncomeButton, &QPushButton::clicked,
            this, [this]() { addRecord(true); });
    connect(editButton, &QPushButton::clicked,
            this, [this]() { editSelectedRecord(); });
    connect(deleteButton, &QPushButton::clicked,
            this, [this]() { deleteSelectedRecord(); });
    connect(exportButton, &QPushButton::clicked,
            this, [this]() { exportCsv(); });
    connect(searchInput, &QLineEdit::textChanged,
            this, [this]() { refreshTransactions(); });
    connect(typeFilter, &QComboBox::currentTextChanged,
            this, [this]() { refreshTransactions(); });
    connect(categoryFilter, &QComboBox::currentTextChanged,
            this, [this]() { refreshTransactions(); });
    connect(sortFilter, &QComboBox::currentTextChanged,
            this, [this]() { refreshTransactions(); });
    connect(clearFiltersButton, &QPushButton::clicked, this, [this]() {
        searchInput->clear();
        typeFilter->setCurrentIndex(0);
        categoryFilter->setCurrentIndex(0);
        sortFilter->setCurrentIndex(0);
        dateFilterCheck->setChecked(false);
        fromDate->setDate(QDate::currentDate().addYears(-5));
        toDate->setDate(QDate::currentDate());
        refreshTransactions();
    });
    connect(transactionTable, &QTableWidget::cellDoubleClicked,
            this, [this](int, int) { editSelectedRecord(); });
    connect(dateFilterCheck, &QCheckBox::toggled,
            this, [this](bool enabled) {
                fromDate->setEnabled(enabled);
                toDate->setEnabled(enabled);
                refreshTransactions();
            });
    connect(fromDate, &QDateEdit::dateChanged,
            this, [this]() { refreshTransactions(); });
    connect(toDate, &QDateEdit::dateChanged,
            this, [this]() { refreshTransactions(); });

    return page;
}

QWidget* MainWindow::buildBudgetsPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 25, 28, 25);
    layout->setSpacing(16);

    auto* header = new QHBoxLayout();
    auto* titleBox = new QVBoxLayout();
    auto* title = new QLabel("Monthly budgets");
    title->setObjectName("pageTitle");
    auto* subtitle = new QLabel("Plan category limits and spot overspending before it grows");
    subtitle->setObjectName("pageSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    addBudgetButton = new QPushButton("+ Create budget");
    addBudgetButton->setObjectName("primaryButton");
    header->addLayout(titleBox);
    header->addStretch();
    header->addWidget(addBudgetButton);
    layout->addLayout(header);

    auto* filterRow = new QHBoxLayout();
    auto* monthLabel = new QLabel("View month:");
    budgetMonthFilter = new QDateEdit(QDate::currentDate());
    budgetMonthFilter->setCalendarPopup(true);
    budgetMonthFilter->setDisplayFormat("MMMM yyyy");
    budgetMonthFilter->setMinimumDate(QDate(2000, 1, 1));
    budgetMonthFilter->setMaximumDate(QDate(2100, 12, 31));
    budgetMonthFilter->setMinimumWidth(190);
    budgetMonthFilter->setToolTip("Choose the month whose category budgets you want to review");
    auto* help = new QLabel("Budgets are tracked against expense records in the selected month.");
    help->setObjectName("mutedText");
    filterRow->addWidget(monthLabel);
    filterRow->addWidget(budgetMonthFilter);
    filterRow->addStretch();
    filterRow->addWidget(help);
    layout->addLayout(filterRow);

    budgetTable = new QTableWidget();
    budgetTable->setColumnCount(8);
    budgetTable->setHorizontalHeaderLabels({
        "Month", "Category", "Budget limit (BDT)", "Spent (BDT)",
        "Remaining (BDT)", "Usage", "Status", "Budget ID"
    });
    budgetTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    budgetTable->setColumnHidden(7, true);
    budgetTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    budgetTable->setSelectionMode(QAbstractItemView::SingleSelection);
    budgetTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    budgetTable->setAlternatingRowColors(true);
    budgetTable->verticalHeader()->setVisible(false);
    layout->addWidget(budgetTable, 1);

    auto* footer = new QHBoxLayout();
    budgetCountLabel = new QLabel("0 budgets");
    editBudgetButton = new QPushButton("Edit selected");
    editBudgetButton->setObjectName("secondaryButton");
    deleteBudgetButton = new QPushButton("Delete selected");
    deleteBudgetButton->setObjectName("dangerButton");
    footer->addWidget(budgetCountLabel);
    footer->addStretch();
    footer->addWidget(editBudgetButton);
    footer->addWidget(deleteBudgetButton);
    layout->addLayout(footer);

    connect(addBudgetButton, &QPushButton::clicked,
            this, [this]() { addBudget(); });
    connect(editBudgetButton, &QPushButton::clicked,
            this, [this]() { editSelectedBudget(); });
    connect(deleteBudgetButton, &QPushButton::clicked,
            this, [this]() { deleteSelectedBudget(); });
    connect(budgetMonthFilter, &QDateEdit::dateChanged,
            this, [this]() { refreshBudgets(); });
    connect(budgetTable, &QTableWidget::cellDoubleClicked,
            this, [this](int, int) { editSelectedBudget(); });

    return page;
}

QWidget* MainWindow::buildRecurringPage()
{
    auto* page = new QWidget();
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(28, 25, 28, 25);
    layout->setSpacing(16);

    auto* header = new QHBoxLayout();
    auto* titleBox = new QVBoxLayout();
    auto* title = new QLabel("Recurring transactions");
    title->setObjectName("pageTitle");
    auto* subtitle = new QLabel("Automate regular bills, subscriptions, salary, and other repeating records");
    subtitle->setObjectName("pageSubtitle");
    titleBox->addWidget(title);
    titleBox->addWidget(subtitle);
    addRecurringButton = new QPushButton("+ New rule");
    addRecurringButton->setObjectName("primaryButton");
    processRecurringButton = new QPushButton("Process due now");
    processRecurringButton->setObjectName("incomeButton");
    header->addLayout(titleBox);
    header->addStretch();
    header->addWidget(processRecurringButton);
    header->addWidget(addRecurringButton);
    layout->addLayout(header);

    auto* hint = new QLabel(
        "Active rules generate dated transactions when the app starts or when you choose Process due now. "
        "Generated records are protected against duplicate creation.");
    hint->setObjectName("mutedText");
    hint->setWordWrap(true);
    layout->addWidget(hint);

    recurringTable = new QTableWidget();
    recurringTable->setColumnCount(8);
    recurringTable->setHorizontalHeaderLabels({
        "Type", "Category / source", "Amount (BDT)", "Frequency",
        "Next due", "Description", "Status", "Rule ID"
    });
    recurringTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    recurringTable->setColumnHidden(7, true);
    recurringTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    recurringTable->setSelectionMode(QAbstractItemView::SingleSelection);
    recurringTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recurringTable->setAlternatingRowColors(true);
    recurringTable->verticalHeader()->setVisible(false);
    layout->addWidget(recurringTable, 1);

    auto* footer = new QHBoxLayout();
    recurringCountLabel = new QLabel("0 recurring rules");
    editRecurringButton = new QPushButton("Edit selected");
    editRecurringButton->setObjectName("secondaryButton");
    toggleRecurringButton = new QPushButton("Pause / Resume");
    toggleRecurringButton->setObjectName("secondaryButton");
    deleteRecurringButton = new QPushButton("Delete selected");
    deleteRecurringButton->setObjectName("dangerButton");
    footer->addWidget(recurringCountLabel);
    footer->addStretch();
    footer->addWidget(editRecurringButton);
    footer->addWidget(toggleRecurringButton);
    footer->addWidget(deleteRecurringButton);
    layout->addLayout(footer);

    connect(addRecurringButton, &QPushButton::clicked,
            this, [this]() { addRecurringRule(); });
    connect(processRecurringButton, &QPushButton::clicked,
            this, [this]() { processDueRecurring(true); });
    connect(editRecurringButton, &QPushButton::clicked,
            this, [this]() { editSelectedRecurringRule(); });
    connect(toggleRecurringButton, &QPushButton::clicked,
            this, [this]() { toggleSelectedRecurringRule(); });
    connect(deleteRecurringButton, &QPushButton::clicked,
            this, [this]() { deleteSelectedRecurringRule(); });
    connect(recurringTable, &QTableWidget::cellDoubleClicked,
            this, [this](int, int) { editSelectedRecurringRule(); });
    return page;
}

void MainWindow::navigateTo(int page)
{
    pageStack->setCurrentIndex(page);
    if (pageFadeAnimation && pageOpacityEffect) {
        pageFadeAnimation->stop();
        pageFadeAnimation->setStartValue(0.62);
        pageFadeAnimation->setEndValue(1.0);
        pageFadeAnimation->start();
    }
    dashboardNavButton->setChecked(page == 0);
    transactionsNavButton->setChecked(page == 1);
    budgetsNavButton->setChecked(page == 2);
    recurringNavButton->setChecked(page == 3);
    if (page == 0) refreshDashboard();
    else if (page == 1) refreshTransactions();
    else if (page == 2) refreshBudgets();
    else refreshRecurring();
}

void MainWindow::refreshAll()
{
    refreshDashboard();
    refreshTransactions();
    refreshBudgets();
    refreshRecurring();
}

void MainWindow::refreshDashboard()
{
    double totalIncome = 0.0;
    double totalExpense = 0.0;
    double currentMonthExpense = 0.0;
    const QDate today = QDate::currentDate();

    for (const Income& income : incomes)
        totalIncome += income.getAmount();

    for (const Expense& expense : expenses) {
        totalExpense += expense.getAmount();
        if (expense.getDate().year() == today.year() &&
            expense.getDate().month() == today.month())
            currentMonthExpense += expense.getAmount();
    }

    totalIncomeValue->setText(moneyText(totalIncome));
    totalExpenseValue->setText(moneyText(totalExpense));
    balanceValue->setText(moneyText(totalIncome - totalExpense));
    monthExpenseValue->setText(moneyText(currentMonthExpense));
    balanceValue->setStyleSheet(totalIncome - totalExpense < 0.0
        ? "color: #dc2626;" : "color: #15803d;");

    QStringList labels;
    QVector<double> monthlyIncome;
    QVector<double> monthlyExpense;
    for (int offset = 5; offset >= 0; --offset) {
        const QDate month = today.addMonths(-offset);
        const int year = month.year();
        const int monthNumber = month.month();
        labels.append(month.toString("MMM"));
        double inTotal = 0.0;
        double outTotal = 0.0;
        for (const Income& income : incomes) {
            if (income.getDate().year() == year && income.getDate().month() == monthNumber)
                inTotal += income.getAmount();
        }
        for (const Expense& expense : expenses) {
            if (expense.getDate().year() == year && expense.getDate().month() == monthNumber)
                outTotal += expense.getAmount();
        }
        monthlyIncome.append(inTotal);
        monthlyExpense.append(outTotal);
    }
    trendChart->setData(labels, monthlyIncome, monthlyExpense);

    QMap<QString, double> categoryTotals;
    for (const Expense& expense : expenses) {
        if (expense.getDate().year() == today.year() &&
            expense.getDate().month() == today.month())
            categoryTotals[expense.getCategory()] += expense.getAmount();
    }

    QVector<QPair<QString, double>> categoryData;
    for (auto it = categoryTotals.cbegin(); it != categoryTotals.cend(); ++it)
        categoryData.append(qMakePair(it.key(), it.value()));
    std::sort(categoryData.begin(), categoryData.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });
    categoryChart->setData(categoryData);

    double monthlyBudgetTotal = 0.0;
    double monthlyBudgetedSpend = 0.0;
    for (const Budget& budget : budgets) {
        if (budget.getMonth() == today.toString("yyyy-MM")) {
            monthlyBudgetTotal += budget.getLimitAmount();
            monthlyBudgetedSpend += spentForBudget(budget);
        }
    }
    if (monthlyBudgetTotal <= 0.0) {
        budgetSummaryLabel->setText("No budgets set for this month");
        dashboardBudgetProgress->setValue(0);
        dashboardBudgetProgress->setFormat("Create category budgets to track your limits");
        dashboardBudgetProgress->setStyleSheet(QString());
    } else {
        const double usedPercent = (monthlyBudgetedSpend / monthlyBudgetTotal) * 100.0;
        budgetSummaryLabel->setText(
            QString("%1 of %2 used · %3 remaining")
                .arg(moneyText(monthlyBudgetedSpend), moneyText(monthlyBudgetTotal),
                     moneyText(monthlyBudgetTotal - monthlyBudgetedSpend)));
        dashboardBudgetProgress->setValue(qBound(0, qRound(usedPercent), 100));
        dashboardBudgetProgress->setFormat(
            QString("%1% of total monthly budget used").arg(usedPercent, 0, 'f', 1));
        if (usedPercent > 100.0) {
            dashboardBudgetProgress->setStyleSheet(
                "QProgressBar::chunk { background: #dc2626; border-radius: 5px; }");
        } else if (usedPercent >= 80.0) {
            dashboardBudgetProgress->setStyleSheet(
                "QProgressBar::chunk { background: #f59e0b; border-radius: 5px; }");
        } else {
            dashboardBudgetProgress->setStyleSheet(QString());
        }
    }
}

void MainWindow::refreshTransactions()
{
    if (!transactionTable) return;
    transactionTable->setRowCount(0);

    QList<TableRecord> records;
    records.reserve(expenses.size() + incomes.size());
    for (const Expense& item : expenses)
        records.append({item.getId(), "Expense", item.getDate(),
                        item.getCategory(), item.getAmount(), item.getNote()});
    for (const Income& item : incomes)
        records.append({item.getId(), "Income", item.getDate(),
                        item.getCategory(), item.getAmount(), item.getNote()});

    const QString sortMode = sortFilter->currentText();
    std::sort(records.begin(), records.end(), [&](const TableRecord& a, const TableRecord& b) {
        if (sortMode == "Oldest first") {
            if (a.date != b.date) return a.date < b.date;
        } else if (sortMode == "Amount: high to low") {
            if (a.amount != b.amount) return a.amount > b.amount;
            if (a.date != b.date) return a.date > b.date;
        } else if (sortMode == "Amount: low to high") {
            if (a.amount != b.amount) return a.amount < b.amount;
            if (a.date != b.date) return a.date > b.date;
        } else {
            if (a.date != b.date) return a.date > b.date;
        }
        return a.type < b.type;
    });

    if (dateFilterCheck->isChecked() && fromDate->date() > toDate->date()) {
        visibleCountLabel->setText("Invalid date range: start date must be on or before end date");
        return;
    }

    const QString search = searchInput->text().trimmed();
    const QString selectedType = typeFilter->currentText();
    const QString previousCategory = categoryFilter->currentText();

    QSet<QString> allCategories;
    for (const Expense& item : expenses) allCategories.insert(item.getCategory());
    for (const Income& item : incomes) allCategories.insert(item.getCategory());
    QStringList categoryNames = allCategories.values();
    categoryNames.sort(Qt::CaseInsensitive);
    categoryFilter->blockSignals(true);
    categoryFilter->clear();
    categoryFilter->addItem("All categories");
    categoryFilter->addItems(categoryNames);
    const int previousIndex = categoryFilter->findText(previousCategory);
    categoryFilter->setCurrentIndex(previousIndex >= 0 ? previousIndex : 0);
    categoryFilter->blockSignals(false);
    const QString selectedCategory = categoryFilter->currentText();
    int visible = 0;

    for (const TableRecord& record : records) {
        if (selectedType != "All types" && record.type != selectedType)
            continue;
        if (selectedCategory != "All categories" && record.category != selectedCategory)
            continue;
        if (!search.isEmpty() &&
            !record.category.contains(search, Qt::CaseInsensitive) &&
            !record.note.contains(search, Qt::CaseInsensitive))
            continue;
        if (dateFilterCheck->isChecked() &&
            (record.date < fromDate->date() || record.date > toDate->date()))
            continue;

        const int row = transactionTable->rowCount();
        transactionTable->insertRow(row);
        transactionTable->setItem(row, 0, new QTableWidgetItem(record.date.toString("dd MMM yyyy")));
        auto* typeItem = new QTableWidgetItem(record.type);
        typeItem->setForeground(record.type == "Income"
            ? QBrush(QColor("#15803d")) : QBrush(QColor("#b91c1c")));
        transactionTable->setItem(row, 1, typeItem);
        transactionTable->setItem(row, 2, new QTableWidgetItem(record.category));
        auto* amountItem = new QTableWidgetItem(QString::number(record.amount, 'f', 2));
        amountItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        transactionTable->setItem(row, 3, amountItem);
        transactionTable->setItem(row, 4, new QTableWidgetItem(record.note));
        auto* idItem = new QTableWidgetItem(record.id);
        idItem->setData(Qt::UserRole, record.type);
        transactionTable->setItem(row, 5, idItem);
        ++visible;
    }

    visibleCountLabel->setText(QString("Showing %1 of %2 transactions")
        .arg(visible).arg(records.size()));
}

double MainWindow::spentForBudget(const Budget& budget) const
{
    double spent = 0.0;
    for (const Expense& expense : expenses) {
        if (expense.getDate().toString("yyyy-MM") == budget.getMonth() &&
            expense.getCategory().compare(budget.getCategory(), Qt::CaseInsensitive) == 0) {
            spent += expense.getAmount();
        }
    }
    return spent;
}

void MainWindow::refreshBudgets()
{
    if (!budgetTable || !budgetMonthFilter) return;

    budgetTable->setRowCount(0);
    const QString selectedMonth = budgetMonthFilter->date().toString("yyyy-MM");

    QList<Budget> monthBudgets;
    for (const Budget& budget : budgets) {
        if (budget.getMonth() == selectedMonth) monthBudgets.append(budget);
    }
    std::sort(monthBudgets.begin(), monthBudgets.end(),
              [](const Budget& a, const Budget& b) {
                  return a.getCategory().compare(b.getCategory(), Qt::CaseInsensitive) < 0;
              });

    double totalLimit = 0.0;
    double totalSpent = 0.0;
    for (const Budget& budget : monthBudgets) {
        const double spent = spentForBudget(budget);
        const double remaining = budget.getLimitAmount() - spent;
        const double percent = (spent / budget.getLimitAmount()) * 100.0;
        totalLimit += budget.getLimitAmount();
        totalSpent += spent;

        const int row = budgetTable->rowCount();
        budgetTable->insertRow(row);
        budgetTable->setItem(row, 0, new QTableWidgetItem(
            QDate::fromString(budget.getMonth() + "-01", Qt::ISODate).toString("MMM yyyy")));
        budgetTable->setItem(row, 1, new QTableWidgetItem(budget.getCategory()));

        auto* limitItem = new QTableWidgetItem(QString::number(budget.getLimitAmount(), 'f', 2));
        limitItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        budgetTable->setItem(row, 2, limitItem);
        auto* spentItem = new QTableWidgetItem(QString::number(spent, 'f', 2));
        spentItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        budgetTable->setItem(row, 3, spentItem);
        auto* remainingItem = new QTableWidgetItem(QString::number(remaining, 'f', 2));
        remainingItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        remainingItem->setForeground(remaining < 0.0 ? QBrush(QColor("#dc2626"))
                                                     : QBrush(QColor("#15803d")));
        budgetTable->setItem(row, 4, remainingItem);

        auto* progress = new QProgressBar();
        progress->setRange(0, 100);
        progress->setValue(qBound(0, qRound(percent), 100));
        progress->setFormat(QString("%1%").arg(percent, 0, 'f', 1));
        if (percent > 100.0) {
            progress->setStyleSheet("QProgressBar::chunk { background: #dc2626; border-radius: 5px; }");
        } else if (percent >= 80.0) {
            progress->setStyleSheet("QProgressBar::chunk { background: #f59e0b; border-radius: 5px; }");
        }
        budgetTable->setCellWidget(row, 5, progress);

        QString status;
        QColor statusColor;
        if (percent > 100.0) {
            status = "Over budget";
            statusColor = QColor("#b91c1c");
        } else if (percent >= 80.0) {
            status = "Near limit";
            statusColor = QColor("#b45309");
        } else {
            status = "On track";
            statusColor = QColor("#15803d");
        }
        auto* statusItem = new QTableWidgetItem(status);
        statusItem->setForeground(QBrush(statusColor));
        budgetTable->setItem(row, 6, statusItem);
        budgetTable->setItem(row, 7, new QTableWidgetItem(budget.getId()));
        budgetTable->setRowHeight(row, 34);
    }

    budgetCountLabel->setText(QString("%1 budgets · %2 limit · %3 spent")
        .arg(monthBudgets.size()).arg(moneyText(totalLimit), moneyText(totalSpent)));
}

bool MainWindow::saveBudgetData()
{
    QString error;
    if (!budgetStorage.save(budgets, &error)) {
        QMessageBox::critical(this, "Budget save failed", error);
        return false;
    }
    budgetStorageReady = true;
    return true;
}

void MainWindow::addBudget()
{
    if (!budgetStorageReady) {
        QMessageBox::warning(this, "Budget storage unavailable",
                             "Budget data could not be loaded. Restore or repair it before adding budgets.");
        return;
    }

    QSet<QString> known;
    for (const Expense& item : expenses) known.insert(item.getCategory());
    for (const Income& item : incomes) known.insert(item.getCategory());
    for (const Budget& item : budgets) known.insert(item.getCategory());
    QStringList categories = known.values();
    categories.sort(Qt::CaseInsensitive);

    BudgetDialog dialog(this, nullptr, categories);
    if (dialog.exec() != QDialog::Accepted) return;

    const QString month = dialog.month();
    const QString category = dialog.category();
    const auto duplicate = std::find_if(budgets.cbegin(), budgets.cend(),
        [&](const Budget& item) {
            return item.getMonth() == month &&
                item.getCategory().compare(category, Qt::CaseInsensitive) == 0;
        });
    if (duplicate != budgets.cend()) {
        QMessageBox::warning(this, "Budget already exists",
            "A budget for this category and month already exists. Edit the existing budget instead.");
        return;
    }

    budgets.append(Budget(QUuid::createUuid().toString(QUuid::WithoutBraces),
                          month, category, dialog.amount()));
    if (!saveBudgetData()) {
        budgets.removeLast();
        return;
    }
    refreshAll();
}

void MainWindow::editSelectedBudget()
{
    const int row = budgetTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select a budget", "Select a budget to edit.");
        return;
    }
    const auto* idItem = budgetTable->item(row, 7);
    if (!idItem) return;
    const QString id = idItem->text();
    auto it = std::find_if(budgets.begin(), budgets.end(),
        [&](const Budget& item) { return item.getId() == id; });
    if (it == budgets.end()) return;

    QSet<QString> known;
    for (const Expense& item : expenses) known.insert(item.getCategory());
    for (const Income& item : incomes) known.insert(item.getCategory());
    for (const Budget& item : budgets) known.insert(item.getCategory());
    QStringList categories = known.values();
    categories.sort(Qt::CaseInsensitive);

    const Budget previous = *it;
    BudgetDialog dialog(this, &previous, categories);
    if (dialog.exec() != QDialog::Accepted) return;

    const auto duplicate = std::find_if(budgets.cbegin(), budgets.cend(),
        [&](const Budget& item) {
            return item.getId() != id && item.getMonth() == dialog.month() &&
                item.getCategory().compare(dialog.category(), Qt::CaseInsensitive) == 0;
        });
    if (duplicate != budgets.cend()) {
        QMessageBox::warning(this, "Budget already exists",
            "Another budget already uses this category and month.");
        return;
    }

    it->setMonth(dialog.month());
    it->setCategory(dialog.category());
    it->setLimitAmount(dialog.amount());
    if (!saveBudgetData()) { *it = previous; return; }
    refreshAll();
}

void MainWindow::deleteSelectedBudget()
{
    const int row = budgetTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select a budget", "Select a budget to delete.");
        return;
    }
    const auto* idItem = budgetTable->item(row, 7);
    if (!idItem) return;
    const QString id = idItem->text();
    const auto it = std::find_if(budgets.begin(), budgets.end(),
        [&](const Budget& item) { return item.getId() == id; });
    if (it == budgets.end()) return;

    if (QMessageBox::question(this, "Delete budget",
        "Delete this monthly category budget? Existing transactions will not be deleted.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    const int index = static_cast<int>(std::distance(budgets.begin(), it));
    const Budget previous = budgets.at(index);
    budgets.removeAt(index);
    if (!saveBudgetData()) { budgets.insert(index, previous); return; }
    refreshAll();
}

void MainWindow::refreshRecurring()
{
    if (!recurringTable || !recurringCountLabel) return;

    recurringTable->setRowCount(0);
    QList<RecurringRule> sorted = recurringRules;
    std::sort(sorted.begin(), sorted.end(), [](const RecurringRule& left, const RecurringRule& right) {
        if (left.isActive() != right.isActive()) return left.isActive();
        return left.getNextDate() < right.getNextDate();
    });

    int activeCount = 0;
    for (const RecurringRule& rule : sorted) {
        const int row = recurringTable->rowCount();
        recurringTable->insertRow(row);
        recurringTable->setItem(row, 0, new QTableWidgetItem(rule.getType()));
        recurringTable->setItem(row, 1, new QTableWidgetItem(rule.getCategory()));
        recurringTable->setItem(row, 2, new QTableWidgetItem(QString::number(rule.getAmount(), 'f', 2)));
        recurringTable->setItem(row, 3, new QTableWidgetItem(rule.getFrequency()));
        recurringTable->setItem(row, 4, new QTableWidgetItem(rule.getNextDate().toString("dd MMM yyyy")));
        recurringTable->setItem(row, 5, new QTableWidgetItem(rule.getNote()));
        const bool dueNow = rule.isActive() && rule.getNextDate() <= QDate::currentDate();
        const QString statusText = !rule.isActive() ? "Paused" : (dueNow ? "Due now" : "Active");
        auto* status = new QTableWidgetItem(statusText);
        const QColor statusColor = !rule.isActive() ? QColor("#64748b")
            : (dueNow ? QColor("#b45309") : QColor("#15803d"));
        status->setForeground(QBrush(statusColor));
        recurringTable->setItem(row, 6, status);
        recurringTable->setItem(row, 7, new QTableWidgetItem(rule.getId()));
        recurringTable->setRowHeight(row, 34);
        if (rule.isActive()) ++activeCount;
    }

    recurringCountLabel->setText(QString("%1 rules · %2 active · %3 paused")
        .arg(recurringRules.size()).arg(activeCount).arg(recurringRules.size() - activeCount));
}

bool MainWindow::saveRecurringData()
{
    QString error;
    if (!recurringStorage.save(recurringRules, &error)) {
        QMessageBox::critical(this, "Recurring rules save failed", error);
        return false;
    }
    recurringStorageReady = true;
    return true;
}

void MainWindow::processDueRecurring(bool showSummary)
{
    if (!expenseStorageReady || !incomeStorageReady || !recurringStorageReady) {
        if (showSummary) {
            QMessageBox::warning(this, "Recurring transactions unavailable",
                "Expense, income, or recurring-rule data could not be loaded. Repair the data files first.");
        }
        return;
    }

    const QList<Expense> oldExpenses = expenses;
    const QList<Income> oldIncomes = incomes;
    const QList<RecurringRule> oldRules = recurringRules;
    const QDate today = QDate::currentDate();
    bool expensesChanged = false;
    bool incomesChanged = false;
    bool rulesChanged = false;
    int createdCount = 0;
    int cappedRules = 0;

    auto expenseIdExists = [&](const QString& id) {
        return std::any_of(expenses.cbegin(), expenses.cend(), [&](const Expense& item) {
            return item.getId() == id;
        });
    };
    auto incomeIdExists = [&](const QString& id) {
        return std::any_of(incomes.cbegin(), incomes.cend(), [&](const Income& item) {
            return item.getId() == id;
        });
    };

    for (RecurringRule& rule : recurringRules) {
        if (!rule.isActive() || rule.getNextDate() > today) continue;

        QDate dueDate = rule.getNextDate();
        int processedForRule = 0;
        while (dueDate.isValid() && dueDate <= today && processedForRule < 240) {
            // A deterministic ID makes processing idempotent if a save is interrupted.
            const QString generatedId = rule.transactionIdForDate(dueDate);

            if (rule.getType() == "Expense") {
                if (!expenseIdExists(generatedId)) {
                    expenses.append(Expense(generatedId, dueDate, rule.getCategory(),
                                            rule.getAmount(), rule.getNote()));
                    expensesChanged = true;
                    ++createdCount;
                }
            } else {
                if (!incomeIdExists(generatedId)) {
                    incomes.append(Income(generatedId, dueDate, rule.getCategory(),
                                          rule.getAmount(), rule.getNote()));
                    incomesChanged = true;
                    ++createdCount;
                }
            }

            const QDate nextDate = rule.nextOccurrenceAfter(dueDate);
            if (!nextDate.isValid() || nextDate <= dueDate) break;
            dueDate = nextDate;
            ++processedForRule;
        }

        if (dueDate > rule.getNextDate()) {
            rule.setNextDate(dueDate);
            rulesChanged = true;
        }
        if (processedForRule >= 240 && dueDate <= today) ++cappedRules;
    }

    QString saveError;
    if (expensesChanged && !expenseStorage.save(expenses, &saveError)) {
        expenses = oldExpenses;
        incomes = oldIncomes;
        recurringRules = oldRules;
        QMessageBox::critical(this, "Recurring processing failed",
            "Could not save generated expenses. No recurring changes were kept.\n\n" + saveError);
        refreshAll();
        return;
    }
    if (incomesChanged && !incomeStorage.save(incomes, &saveError)) {
        expenses = oldExpenses;
        incomes = oldIncomes;
        recurringRules = oldRules;
        QString rollbackError;
        if (expensesChanged) expenseStorage.save(oldExpenses, &rollbackError);
        QMessageBox::critical(this, "Recurring processing failed",
            "Could not save generated income. The app attempted to roll back generated expenses.\n\n" + saveError);
        refreshAll();
        return;
    }
    if (rulesChanged && !recurringStorage.save(recurringRules, &saveError)) {
        // Transactions are already saved. Deterministic IDs prevent duplicates when retried.
        recurringRules = oldRules;
        QMessageBox::warning(this, "Recurring rules were not advanced",
            "Transactions were saved, but the next-due dates could not be saved. Processing again will not duplicate transactions.\n\n" + saveError);
    }

    refreshAll();
    if (showSummary) {
        QString message = createdCount == 0
            ? "There were no due recurring transactions to create."
            : QString("Created %1 recurring transaction(s).").arg(createdCount);
        if (cappedRules > 0) {
            message += QString("\n%1 rule(s) had more than 240 overdue occurrences; process again to continue catching up.").arg(cappedRules);
        }
        QMessageBox::information(this, "Recurring processing complete", message);
    }
}

void MainWindow::addRecurringRule()
{
    if (!recurringStorageReady || !expenseStorageReady || !incomeStorageReady) {
        QMessageBox::warning(this, "Recurring rules unavailable",
            "Load expense, income and recurring data successfully before adding a rule.");
        return;
    }

    QSet<QString> known;
    known.insert("Food"); known.insert("Transport"); known.insert("Education");
    known.insert("Shopping"); known.insert("Bills"); known.insert("Health");
    known.insert("Entertainment"); known.insert("Other");
    for (const Expense& item : expenses) known.insert(item.getCategory());
    for (const Income& item : incomes) known.insert(item.getCategory());
    for (const Budget& item : budgets) known.insert(item.getCategory());
    QStringList categories = known.values();
    categories.sort(Qt::CaseInsensitive);

    RecurringDialog dialog(this, nullptr, categories);
    if (dialog.exec() != QDialog::Accepted) return;

    recurringRules.append(RecurringRule(
        QUuid::createUuid().toString(QUuid::WithoutBraces),
        dialog.type(), dialog.category(), dialog.amount(),
        dialog.frequency(), dialog.nextDate(), dialog.note(), true));
    if (!saveRecurringData()) {
        recurringRules.removeLast();
        return;
    }
    refreshAll();
}

void MainWindow::editSelectedRecurringRule()
{
    const int row = recurringTable->currentRow();
    if (row < 0) return;
    const QTableWidgetItem* idItem = recurringTable->item(row, 7);
    if (!idItem) return;
    const QString id = idItem->text();
    auto it = std::find_if(recurringRules.begin(), recurringRules.end(), [&](const RecurringRule& rule) {
        return rule.getId() == id;
    });
    if (it == recurringRules.end()) return;

    QSet<QString> known;
    for (const Expense& item : expenses) known.insert(item.getCategory());
    for (const Income& item : incomes) known.insert(item.getCategory());
    for (const Budget& item : budgets) known.insert(item.getCategory());
    QStringList categories = known.values();
    categories.sort(Qt::CaseInsensitive);

    const RecurringRule previous = *it;
    RecurringDialog dialog(this, &previous, categories);
    if (dialog.exec() != QDialog::Accepted) return;

    it->setType(dialog.type());
    it->setCategory(dialog.category());
    it->setAmount(dialog.amount());
    it->setFrequency(dialog.frequency());
    if (dialog.nextDate() != previous.getNextDate() ||
        (dialog.frequency() == "Monthly" && previous.getFrequency() != "Monthly"))
        it->resetScheduleStartDate(dialog.nextDate());
    else
        it->setNextDate(dialog.nextDate());
    it->setNote(dialog.note());
    if (!saveRecurringData()) { *it = previous; return; }
    refreshAll();
}

void MainWindow::toggleSelectedRecurringRule()
{
    const int row = recurringTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select a rule", "Select a recurring rule first.");
        return;
    }
    const QTableWidgetItem* idItem = recurringTable->item(row, 7);
    if (!idItem) return;
    const QString id = idItem->text();
    auto it = std::find_if(recurringRules.begin(), recurringRules.end(), [&](const RecurringRule& rule) {
        return rule.getId() == id;
    });
    if (it == recurringRules.end()) return;
    const bool oldState = it->isActive();
    it->setActive(!oldState);
    if (!saveRecurringData()) { it->setActive(oldState); return; }
    refreshRecurring();
}

void MainWindow::deleteSelectedRecurringRule()
{
    const int row = recurringTable->currentRow();
    if (row < 0) {
        QMessageBox::information(this, "Select a rule", "Select a recurring rule first.");
        return;
    }
    const QTableWidgetItem* idItem = recurringTable->item(row, 7);
    if (!idItem) return;
    const QString id = idItem->text();
    auto it = std::find_if(recurringRules.begin(), recurringRules.end(), [&](const RecurringRule& rule) {
        return rule.getId() == id;
    });
    if (it == recurringRules.end()) return;
    if (QMessageBox::question(this, "Delete recurring rule",
        "Delete this rule? Transactions already generated by it will remain in your history.",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;

    const int index = static_cast<int>(std::distance(recurringRules.begin(), it));
    const RecurringRule previous = recurringRules.at(index);
    recurringRules.removeAt(index);
    if (!saveRecurringData()) { recurringRules.insert(index, previous); return; }
    refreshAll();
}

bool MainWindow::writeBackupFile(const QString& path, QString* errorMessage) const
{
    auto fail = [&](const QString& message) {
        if (errorMessage) *errorMessage = message;
        return false;
    };

    QJsonArray expenseArray;
    for (const Expense& item : expenses) expenseArray.append(item.toJson());
    QJsonArray incomeArray;
    for (const Income& item : incomes) incomeArray.append(item.toJson());
    QJsonArray budgetArray;
    for (const Budget& item : budgets) budgetArray.append(item.toJson());
    QJsonArray recurringArray;
    for (const RecurringRule& item : recurringRules) recurringArray.append(item.toJson());

    QJsonObject root;
    root.insert("format", "personal-finance-tracker-backup");
    root.insert("schemaVersion", 2);
    root.insert("applicationVersion", "3.0.0");
    root.insert("createdAt", QDateTime::currentDateTime().toString(Qt::ISODateWithMs));
    root.insert("expenses", expenseArray);
    root.insert("incomes", incomeArray);
    root.insert("budgets", budgetArray);
    root.insert("recurringRules", recurringArray);

    const QByteArray json = QJsonDocument(root).toJson(QJsonDocument::Indented);
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return fail("Could not create backup: " + file.errorString());
    if (file.write(json) != json.size()) {
        file.cancelWriting();
        return fail("Could not write the complete backup file.");
    }
    if (!file.commit()) return fail("Could not finalize backup: " + file.errorString());
    if (errorMessage) errorMessage->clear();
    return true;
}

void MainWindow::backupData()
{
    if (!expenseStorageReady || !incomeStorageReady || !budgetStorageReady || !recurringStorageReady) {
        QMessageBox::warning(this, "Backup unavailable",
            "One or more data files could not be loaded. Resolve the data warning before creating a complete backup.");
        return;
    }

    const QString suggested = QString("personal-finance-backup-%1.json")
        .arg(QDateTime::currentDateTime().toString("yyyyMMdd-HHmm"));
    QString path = QFileDialog::getSaveFileName(
        this, "Back up personal finance data", suggested, "JSON backup (*.json)");
    if (path.isEmpty()) return;
    if (!path.endsWith(".json", Qt::CaseInsensitive)) path += ".json";

    QString error;
    if (!writeBackupFile(path, &error)) {
        QMessageBox::critical(this, "Backup failed", error);
        return;
    }
    QMessageBox::information(this, "Backup complete",
        QString("Your income, expenses, budgets and recurring rules were backed up to:\n%1\n\nKeep this file in a safe place.").arg(path));
}

void MainWindow::restoreData()
{
    const QString path = QFileDialog::getOpenFileName(
        this, "Restore personal finance backup", QString(), "JSON backup (*.json)");
    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Restore failed", "Could not read the selected backup: " + file.errorString());
        return;
    }
    const QByteArray bytes = file.readAll();
    file.close();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(bytes, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        QMessageBox::critical(this, "Invalid backup", "The selected file is not a valid JSON backup.");
        return;
    }

    const QJsonObject root = document.object();
    const int schemaVersion = root.value("schemaVersion").toInt(-1);
    const bool hasRecurringRules = root.value("recurringRules").isArray();
    if (root.value("format").toString() != "personal-finance-tracker-backup" ||
        (schemaVersion != 1 && schemaVersion != 2) ||
        !root.value("expenses").isArray() || !root.value("incomes").isArray() ||
        !root.value("budgets").isArray() || (schemaVersion >= 2 && !hasRecurringRules)) {
        QMessageBox::critical(this, "Unsupported backup",
            "This file is not a supported Personal Finance Tracker backup.");
        return;
    }

    QList<Expense> restoredExpenses;
    QSet<QString> expenseIds;
    const QJsonArray expenseArray = root.value("expenses").toArray();
    for (qsizetype i = 0; i < expenseArray.size(); ++i) {
        if (!expenseArray.at(i).isObject()) {
            QMessageBox::critical(this, "Invalid backup",
                QString("Expense record %1 is invalid.").arg(i + 1));
            return;
        }
        Expense item;
        QString error;
        if (!Expense::fromJson(expenseArray.at(i).toObject(), item, &error)) {
            QMessageBox::critical(this, "Invalid backup",
                QString("Expense record %1: %2").arg(i + 1).arg(error));
            return;
        }
        if (expenseIds.contains(item.getId())) {
            QMessageBox::critical(this, "Invalid backup", "The backup contains duplicate expense IDs.");
            return;
        }
        expenseIds.insert(item.getId());
        restoredExpenses.append(item);
    }

    QList<Income> restoredIncomes;
    QSet<QString> incomeIds;
    const QJsonArray incomeArray = root.value("incomes").toArray();
    for (qsizetype i = 0; i < incomeArray.size(); ++i) {
        if (!incomeArray.at(i).isObject()) {
            QMessageBox::critical(this, "Invalid backup",
                QString("Income record %1 is invalid.").arg(i + 1));
            return;
        }
        Income item;
        QString error;
        if (!Income::fromJson(incomeArray.at(i).toObject(), item, &error)) {
            QMessageBox::critical(this, "Invalid backup",
                QString("Income record %1: %2").arg(i + 1).arg(error));
            return;
        }
        if (incomeIds.contains(item.getId())) {
            QMessageBox::critical(this, "Invalid backup", "The backup contains duplicate income IDs.");
            return;
        }
        incomeIds.insert(item.getId());
        restoredIncomes.append(item);
    }

    QList<Budget> restoredBudgets;
    const QJsonArray budgetArray = root.value("budgets").toArray();
    QSet<QString> budgetKeys;
    QSet<QString> budgetIds;
    for (qsizetype i = 0; i < budgetArray.size(); ++i) {
        if (!budgetArray.at(i).isObject()) {
            QMessageBox::critical(this, "Invalid backup",
                QString("Budget record %1 is invalid.").arg(i + 1));
            return;
        }
        Budget item;
        QString error;
        if (!Budget::fromJson(budgetArray.at(i).toObject(), item, &error)) {
            QMessageBox::critical(this, "Invalid backup",
                QString("Budget record %1: %2").arg(i + 1).arg(error));
            return;
        }
        if (budgetIds.contains(item.getId())) {
            QMessageBox::critical(this, "Invalid backup", "The backup contains duplicate budget IDs.");
            return;
        }
        budgetIds.insert(item.getId());
        const QString key = item.getMonth() + QChar(0x1f) + item.getCategory().toCaseFolded();
        if (budgetKeys.contains(key)) {
            QMessageBox::critical(this, "Invalid backup", "The backup contains duplicate category/month budgets.");
            return;
        }
        budgetKeys.insert(key);
        restoredBudgets.append(item);
    }

    QList<RecurringRule> restoredRecurringRules;
    if (hasRecurringRules) {
        const QJsonArray recurringArray = root.value("recurringRules").toArray();
        QSet<QString> recurringIds;
        for (qsizetype i = 0; i < recurringArray.size(); ++i) {
            if (!recurringArray.at(i).isObject()) {
                QMessageBox::critical(this, "Invalid backup",
                    QString("Recurring rule %1 is invalid.").arg(i + 1));
                return;
            }
            RecurringRule item;
            QString error;
            if (!RecurringRule::fromJson(recurringArray.at(i).toObject(), item, &error)) {
                QMessageBox::critical(this, "Invalid backup",
                    QString("Recurring rule %1: %2").arg(i + 1).arg(error));
                return;
            }
            if (recurringIds.contains(item.getId())) {
                QMessageBox::critical(this, "Invalid backup", "The backup contains duplicate recurring rule IDs.");
                return;
            }
            recurringIds.insert(item.getId());
            restoredRecurringRules.append(item);
        }
    }

    if (!expenseStorageReady || !incomeStorageReady || !budgetStorageReady || !recurringStorageReady) {
        QMessageBox::warning(this, "Restore unavailable",
            "Current data could not be loaded completely, so a safety backup cannot be guaranteed. Repair or manually back up the data folder first.");
        return;
    }

    if (QMessageBox::warning(this, "Replace current data?",
        "Restoring will replace all current income, expense, budget and recurring-rule records.\n\n"
        "A safety backup of the current data will be created first. Continue?",
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
        return;

    const QString dataDir = QFileInfo(expenseStorage.getFilePath()).absolutePath();
    const QString backupDir = QDir(dataDir).filePath("backups");
    if (!QDir().mkpath(backupDir)) {
        QMessageBox::critical(this, "Restore failed", "Could not create the safety backup folder.");
        return;
    }
    const QString safetyPath = QDir(backupDir).filePath(
        QString("pre-restore-%1.json").arg(QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss")));
    QString backupError;
    if (!writeBackupFile(safetyPath, &backupError)) {
        QMessageBox::critical(this, "Restore cancelled",
            "Could not create the safety backup. Current data was not changed.\n\n" + backupError);
        return;
    }

    const QList<Expense> oldExpenses = expenses;
    const QList<Income> oldIncomes = incomes;
    const QList<Budget> oldBudgets = budgets;
    const QList<RecurringRule> oldRecurringRules = recurringRules;
    expenses = restoredExpenses;
    incomes = restoredIncomes;
    budgets = restoredBudgets;
    recurringRules = restoredRecurringRules;

    QString saveError;
    const bool expenseSaved = expenseStorage.save(expenses, &saveError);
    const bool incomeSaved = expenseSaved && incomeStorage.save(incomes, &saveError);
    const bool budgetSaved = incomeSaved && budgetStorage.save(budgets, &saveError);
    const bool recurringSaved = budgetSaved && recurringStorage.save(recurringRules, &saveError);

    if (!expenseSaved || !incomeSaved || !budgetSaved || !recurringSaved) {
        expenses = oldExpenses;
        incomes = oldIncomes;
        budgets = oldBudgets;
        recurringRules = oldRecurringRules;
        QString rollbackError;
        expenseStorage.save(oldExpenses, &rollbackError);
        incomeStorage.save(oldIncomes, &rollbackError);
        budgetStorage.save(oldBudgets, &rollbackError);
        recurringStorage.save(oldRecurringRules, &rollbackError);
        QMessageBox::critical(this, "Restore failed",
            "The selected backup could not be fully restored. The app attempted to return to the previous data.\n"
            "A pre-restore safety backup is available at:\n" + safetyPath + "\n\nDetails: " + saveError);
        refreshAll();
        return;
    }

    expenseStorageReady = incomeStorageReady = budgetStorageReady = recurringStorageReady = true;
    refreshAll();
    QMessageBox::information(this, "Restore complete",
        QString("Backup restored successfully.\n\nSafety copy of the previous data:\n%1").arg(safetyPath));
}

bool MainWindow::saveExpenseData()
{
    QString error;
    if (!expenseStorage.save(expenses, &error)) {
        QMessageBox::critical(this, "Expense save failed", error);
        return false;
    }
    return true;
}

bool MainWindow::saveIncomeData()
{
    QString error;
    if (!incomeStorage.save(incomes, &error)) {
        QMessageBox::critical(this, "Income save failed", error);
        return false;
    }
    return true;
}

void MainWindow::addRecord(bool isIncome)
{
    if (isIncome ? !incomeStorageReady : !expenseStorageReady) return;

    FinancialRecordDialog dialog(isIncome, this);
    if (dialog.exec() != QDialog::Accepted) return;

    const QString id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    if (isIncome) {
        incomes.append(Income(id, dialog.date(), dialog.category(),
                              dialog.amount(), dialog.note()));
        if (!saveIncomeData()) {
            incomes.removeLast();
            return;
        }
    } else {
        expenses.append(Expense(id, dialog.date(), dialog.category(),
                                dialog.amount(), dialog.note()));
        if (!saveExpenseData()) {
            expenses.removeLast();
            return;
        }
    }

    refreshAll();
}

QString MainWindow::selectedRecordId() const
{
    const int row = transactionTable->currentRow();
    if (row < 0) return QString();
    const QTableWidgetItem* item = transactionTable->item(row, 5);
    return item ? item->text() : QString();
}

bool MainWindow::selectedRecordIsIncome() const
{
    const int row = transactionTable->currentRow();
    if (row < 0) return false;
    const QTableWidgetItem* item = transactionTable->item(row, 5);
    return item && item->data(Qt::UserRole).toString() == "Income";
}

void MainWindow::editSelectedRecord()
{
    const QString id = selectedRecordId();
    if (id.isEmpty()) {
        QMessageBox::information(this, "Select a record", "Select a transaction to edit.");
        return;
    }

    if (selectedRecordIsIncome()) {
        if (!incomeStorageReady) return;
        auto it = std::find_if(incomes.begin(), incomes.end(),
                               [&](const Income& item) { return item.getId() == id; });
        if (it == incomes.end()) return;
        const Income previous = *it;
        FinancialRecordDialog dialog(true, this, nullptr, &previous);
        if (dialog.exec() != QDialog::Accepted) return;
        it->setAmount(dialog.amount());
        it->setCategory(dialog.category());
        it->setDate(dialog.date());
        it->setNote(dialog.note());
        if (!saveIncomeData()) { *it = previous; return; }
    } else {
        if (!expenseStorageReady) return;
        auto it = std::find_if(expenses.begin(), expenses.end(),
                               [&](const Expense& item) { return item.getId() == id; });
        if (it == expenses.end()) return;
        const Expense previous = *it;
        FinancialRecordDialog dialog(false, this, &previous, nullptr);
        if (dialog.exec() != QDialog::Accepted) return;
        it->setAmount(dialog.amount());
        it->setCategory(dialog.category());
        it->setDate(dialog.date());
        it->setNote(dialog.note());
        if (!saveExpenseData()) { *it = previous; return; }
    }

    refreshAll();
}

void MainWindow::deleteSelectedRecord()
{
    const QString id = selectedRecordId();
    if (id.isEmpty()) {
        QMessageBox::information(this, "Select a record", "Select a transaction to delete.");
        return;
    }

    const bool isIncome = selectedRecordIsIncome();
    const auto answer = QMessageBox::question(
        this, "Confirm deletion",
        QString("Delete this %1 record permanently?").arg(isIncome ? "income" : "expense"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes) return;

    if (isIncome) {
        if (!incomeStorageReady) return;
        const auto it = std::find_if(incomes.begin(), incomes.end(),
                                     [&](const Income& item) { return item.getId() == id; });
        if (it == incomes.end()) return;
        const int index = static_cast<int>(std::distance(incomes.begin(), it));
        const Income previous = incomes.at(index);
        incomes.removeAt(index);
        if (!saveIncomeData()) { incomes.insert(index, previous); return; }
    } else {
        if (!expenseStorageReady) return;
        const auto it = std::find_if(expenses.begin(), expenses.end(),
                                     [&](const Expense& item) { return item.getId() == id; });
        if (it == expenses.end()) return;
        const int index = static_cast<int>(std::distance(expenses.begin(), it));
        const Expense previous = expenses.at(index);
        expenses.removeAt(index);
        if (!saveExpenseData()) { expenses.insert(index, previous); return; }
    }
    refreshAll();
}

void MainWindow::exportCsv()
{
    const QString path = QFileDialog::getSaveFileName(
        this, "Export transactions", "transactions.csv",
        "CSV files (*.csv)");
    if (path.isEmpty()) return;

    QList<TableRecord> records;
    for (const Expense& item : expenses)
        records.append({item.getId(), "Expense", item.getDate(),
                        item.getCategory(), item.getAmount(), item.getNote()});
    for (const Income& item : incomes)
        records.append({item.getId(), "Income", item.getDate(),
                        item.getCategory(), item.getAmount(), item.getNote()});
    std::sort(records.begin(), records.end(), [](const TableRecord& a, const TableRecord& b) {
        return a.date > b.date;
    });

    QString csv = "Date,Type,Category,Amount (BDT),Description\r\n";
    for (const TableRecord& record : records) {
        csv += csvEscape(record.date.toString(Qt::ISODate)) + ",";
        csv += csvEscape(record.type) + ",";
        csv += csvEscape(record.category) + ",";
        csv += QString::number(record.amount, 'f', 2) + ",";
        csv += csvEscape(record.note) + "\r\n";
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Export failed", file.errorString());
        return;
    }
    const QByteArray output = QByteArray::fromHex("efbbbf") + csv.toUtf8();
    if (file.write(output) != output.size() || !file.commit()) {
        file.cancelWriting();
        QMessageBox::critical(this, "Export failed",
                              "Could not write the CSV file.");
        return;
    }
    QMessageBox::information(this, "Export complete",
        QString("Exported %1 transactions to:\n%2").arg(records.size()).arg(path));
}
