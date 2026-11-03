# Moving your existing Version 2 data into Version 2.1

Version 2.1 keeps the same expense JSON schema and adds a separate income file.

1. Close the old application.
2. Make a backup copy of the old project's `data/expenses.json`.
3. Extract the Version 2.1 source ZIP into a new folder (or merge it into the existing project without deleting `data/expenses.json`).
4. Copy the backed-up `expenses.json` into the new project's `data` folder.
5. Build and start Version 2.1. The old expense entries should appear in Transactions.
6. Add an income record once; Version 2.1 will create `data/incomes.json` automatically.

The source ZIP intentionally does not contain personal transaction files or generated build output. Keep the JSON files private and do not commit them to GitHub.
