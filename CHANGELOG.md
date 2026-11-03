# Changelog

## [3.0.0] - 2026-11-03

### Added
- Weekly, monthly, and yearly recurring income/expense rules with pause/resume, edit, and delete controls.
- Automatic due-transaction processing at app startup and manual processing from the Recurring page, with deterministic IDs to prevent duplicates on retry.
- Monthly recurrence anchors that preserve the selected day or month-end behavior across shorter months.
- Monthly category budgets with per-month limits, spent/remaining amounts, usage bars, and on-track / near-limit / over-budget states.
- Dashboard budget health summary.
- Full JSON backup and validated restore for expenses, income, budgets, and recurring rules.
- Automatic pre-restore safety backup before replacing data.
- Transaction sorting by date and amount, one-click filter reset, and double-click-to-edit.
- Automated model and storage tests through CTest.
- Sample-data SVG interface previews and expanded setup documentation.

### Improved
- Reject malformed JSON structures and duplicate transaction IDs.
- Reject duplicate category/month budgets.
- Improve keyboard focus, button hover, and budget progress styling.
- Keep personal JSON and backup data out of source packages.

## [2.1.0]

### Added
- Income management and local income JSON storage.
- Financial dashboard with balance, six-month trend, and category chart.
- Transaction search, filters, and CSV export.
- Windows MSYS2 UCRT64 build workflow.
