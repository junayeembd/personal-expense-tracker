# Architecture

The application uses a small layered architecture to keep UI, domain data, and persistence separate.

```text
MainWindow (Qt Widgets UI)
  ├── Expense model  ── ExpenseStorage  ── data/expenses.json
  ├── Income model   ── IncomeStorage   ── data/incomes.json
  └── QPainter chart widgets
```

## Domain models

`Expense` and `Income` each represent one dated monetary record with an ID, category, amount, and optional note. Both validate their data before it is persisted. Existing expense JSON remains in the same shape, so records created by Version 1 continue to load.

## Persistence

Each storage class reads and writes a JSON object containing an array (`expenses` or `incomes`). Writes use `QSaveFile`, which avoids replacing the previous data file until the new content has been fully written and committed.

## Presentation

`MainWindow` owns the two record lists, navigation, dashboard metrics, filtering and record actions. The chart widgets paint simple bar and donut charts using `QPainter`; no Qt Charts dependency is necessary.

## Data safety

- `data/*.json` is ignored by Git because it can contain private financial records.
- Keep a backup of the JSON files before modifying or migrating the project.
- CSV export writes a separate file selected by the user and includes all saved transactions.
