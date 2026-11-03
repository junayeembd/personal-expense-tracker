# Architecture

The application uses a small layered architecture to keep the Qt interface, domain models, and local persistence separate.

```text
MainWindow (Qt Widgets UI)
  ├── Expense model ── ExpenseStorage ── data/expenses.json
  ├── Income model  ── IncomeStorage  ── data/incomes.json
  ├── Budget model  ── BudgetStorage  ── data/budgets.json
  ├── RecurringRule ── RecurringStorage ── data/recurring.json
  ├── QPainter dashboard charts
  └── Versioned backup / validated restore
```

## Domain models

- `Expense` represents a dated outgoing transaction.
- `Income` represents a dated incoming transaction.
- `Budget` represents one category spending limit for one calendar month (`yyyy-MM`).
- `RecurringRule` represents a weekly, monthly, or yearly income/expense rule, including its next due date and active/paused state.

Each model validates IDs, dates, category names, and positive finite amounts before storage. Legacy expense records without an ID are still accepted and receive a generated ID.

## Persistence

Each storage class reads and writes a JSON object containing its own array (`expenses`, `incomes`, `budgets`, or `rules`). Writes use Qt `QSaveFile` so the destination is replaced only after the complete new payload has been written. Storage loading validates the root structure and records before replacing the in-memory list.

## Budget calculations

A budget is uniquely identified by month and category (case-insensitive category comparison). Actual spending is the sum of expense records in that same calendar month and category. Status is:

- **On track:** below 80% usage.
- **Near limit:** from 80% through 100%.
- **Over budget:** above 100%.

A budget is informational and does not block a transaction from being added.

## Backup and restore

A backup is a versioned JSON bundle containing expense, income, budget, and recurring-rule arrays plus metadata. Schema version 1 backups without recurring rules remain restorable; their recurring-rule list is treated as empty. Restore validates every record and duplicate budget key before changing any current data. Before restore, the app writes a pre-restore safety backup. Individual data files are then written with their storage classes' atomic save operations; if a later write fails, the app attempts to roll back to the previous in-memory data.

## Presentation

`MainWindow` owns navigation and page actions. The two charts are small `QPainter` widgets; the project does not require the optional Qt Charts module. Budget progress is rendered using Qt `QProgressBar` widgets. The navigation stack uses a short Qt property-animation fade when switching pages. The Recurring page processes due active rules at startup and on manual request. Generated transaction IDs are deterministic from the rule ID and scheduled date, so retrying after an interrupted save does not duplicate the same occurrence. Monthly rules persist their original day-of-month anchor; a rule scheduled for the 30th returns to the 30th after February, while a rule anchored to month-end stays on each month's final day.

## Privacy

- `data/*.json` contains private transactions and is excluded from Git.
- `data/backups/` contains backup files and is excluded from Git.
- CSV exports can contain private financial information and should not be committed.
