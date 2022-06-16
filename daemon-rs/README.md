# Database changes

We want to create a migration! Get the `diesel` CLI and run

```
diesel migration generate your_migration_name
```

To test it, run

```
diesel migration run --database-url test.db
```

and

```
diesel migration redo --database-url test.db
```