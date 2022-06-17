# Database changes

Install the CLI:

```
cargo install diesel_cli --version 2.0.0-rc.0 --no-default-features --features sqlite --features chrono
```

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
