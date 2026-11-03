# Manual Test Plan — Personal Expense Tracker 2.1

Use disposable test records. Back up your real `data/*.json` files before testing.

| ID | Test | Steps | Expected result |
|---|---|---|---|
| T01 | First launch | Start with no JSON files | Dashboard opens; totals are zero; files are created after first save. |
| T02 | Add expense | Add Food, BDT 260, today's date | Expense appears in Transactions and expense JSON. |
| T03 | Add income | Add Salary, BDT 20,000, today's date | Income appears and is saved in income JSON. |
| T04 | Balance | Add the T02 and T03 records | Balance equals income minus expense. |
| T05 | Edit expense | Select an expense, edit amount/category/date/note | Edited record persists after restart. |
| T06 | Edit income | Select an income, edit its details | Edited income persists after restart. |
| T07 | Delete | Select a record, choose Delete, confirm | Only the selected record is removed after confirmation. |
| T08 | Cancel delete | Select a record and cancel the confirmation | No data changes. |
| T09 | Search | Search by a category or a word in description | Only matching records appear. |
| T10 | Type filter | Select Income, then Expense | Table shows only selected type. |
| T11 | Category filter | Select a category | Table shows records in that category. |
| T12 | Date filter | Enable range and select a date interval | Only records in the inclusive range appear. |
| T13 | CSV export | Export records to a new CSV file and open in Excel | Header and all income/expense rows are readable. |
| T14 | Persistence | Close and reopen the app | All saved records and dashboard totals are restored. |
| T15 | Legacy expense JSON | Keep a Version 1 `expenses.json` and launch | Existing expense entries are loaded; no income records are required. |
| T16 | Empty/invalid amount | Submit blank, zero, or invalid amount | Dialog prevents saving and explains the validation error. |
| T17 | Custom category | Type a custom category in Add dialog | Custom category can be saved and filtered. |
