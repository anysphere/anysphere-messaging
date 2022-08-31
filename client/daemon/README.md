# Rust code

Set up VSCode rust-analyzer (very recommended!):

```
./update-rust-project.sh
```

Then restart the rust-analyzer server in VSCode with F1 and then searching for "Rust analyzer: restart server".

## Database changes

We want to create a migration!

```
./diesel-cli.sh migration generate your_migration_name
```

To test it, run

```
./diesel-cli.sh migration run
```

and

```
./diesel-cli.sh migration redo
```

## Debugging

For debugging the Rust code, run

```
bazelisk test //... --test_env=RUST_BACKTRACE=1
```
