# Rust code

Set up VSCode rust-analyzer (very recommended!):

```
bazelisk run @rules_rust//tools/rust_analyzer:gen_rust_project
```

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
