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
#include <QFont>
#include <QPen>
#include <QBrush>
#include <QVector>
#include <QPair>
#include <QPoint>
#include <QRectF>
#include <QMap>
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
// Main window
// ------------------------------------------------------------

MainWindow::MainWindow(const QString& dataDirectory, QWidget* parent)
    : QMainWindow(parent),
      expenseStorage(QDir(dataDirectory).filePath("expenses.json")),
      incomeStorage(QDir(dataDirectory).filePath("incomes.json"))
{
    setWindowTitle("Personal Finance Tracker | Version 2.1");
    resize(1320, 850);
    setMinimumSize(1050, 700);

    buildUI();

    QString expenseError;
    QString incomeError;
    expenseStorageReady = expenseStorage.load(expenses, &expenseError);
    incomeStorageReady = incomeStorage.load(incomes, &incomeError);

    if (!expenseStorageReady || !incomeStorageReady) {
        QString details;
        if (!expenseStorageReady) details += "Expense data: " + expenseError + "\n";
        if (!incomeStorageReady) details += "Income data: " + incomeError;
        QMessageBox::critical(this, "Data loading error",
                              details + "\n\nThe affected data file could not be loaded. "
                              "Do not overwrite it until it has been checked.");
    }

    addExpenseButton->setEnabled(expenseStorageReady);
    addIncomeButton->setEnabled(incomeStorageReady);
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

    sideLayout->addWidget(dashboardNavButton);
    sideLayout->addWidget(transactionsNavButton);
    sideLayout->addStretch();

    auto* localLabel = new QLabel("LOCAL DATA\nSaved on this device", sidebar);
    localLabel->setObjectName("sidebarFootnote");
    sideLayout->addWidget(localLabel);

    pageStack = new QStackedWidget(central);
    pageStack->addWidget(buildDashboardPage());
    pageStack->addWidget(buildTransactionsPage());

    root->addWidget(sidebar);
    root->addWidget(pageStack, 1);

    central->setStyleSheet(R"(
        QWidget { font-family: "Segoe UI"; font-size: 10pt; color: #1e293b; }
        QWidget#sidebar { background: #101b33; }
        QLabel#brandLabel { color: white; font-size: 17px; font-weight: bold; letter-spacing: 1px; }
        QLabel#sidebarFootnote { color: #a6b3c8; font-size: 9px; }
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
        QPushButton:hover { opacity: 0.9; }
        QTableWidget { background: white; alternate-background-color: #f8fafc;
            gridline-color: #e2e8f0; border: 1px solid #e2e8f0; border-radius: 8px; }
        QHeaderView::section { background: #e8eef7; padding: 11px; border: none; font-weight: bold; }
    )");

    connect(dashboardNavButton, &QPushButton::clicked,
            this, [this]() { navigateTo(0); });
    connect(transactionsNavButton, &QPushButton::clicked,
            this, [this]() { navigateTo(1); });

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
    connect(goTransactions, &QPushButton::clicked,
            this, [this]() { navigateTo(1); });
    header->addLayout(titleBox);
    header->addStretch();
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
    filters->addWidget(dateFilterCheck, 0, 3);
    filters->addWidget(fromDate, 0, 4);
    filters->addWidget(toDate, 0, 5);
    filters->setColumnStretch(0, 3);
    filters->setColumnStretch(1, 1);
    filters->setColumnStretch(2, 2);
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

void MainWindow::navigateTo(int page)
{
    pageStack->setCurrentIndex(page);
    dashboardNavButton->setChecked(page == 0);
    transactionsNavButton->setChecked(page == 1);
    if (page == 0) refreshDashboard();
    else refreshTransactions();
}

void MainWindow::refreshAll()
{
    refreshDashboard();
    refreshTransactions();
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

    std::sort(records.begin(), records.end(), [](const TableRecord& a, const TableRecord& b) {
        if (a.date != b.date) return a.date > b.date;
        return a.type < b.type;
    });

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
        if (dateFilterCheck->isChecked() && fromDate->date() > toDate->date())
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
