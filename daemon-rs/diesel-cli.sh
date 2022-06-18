#!/usr/bin/env bash

if [[ $(diesel --version) != *"diesel 2.0.0-rc.0"* ]]; then
    echo "Installing Diesel CLI..."
    cargo install diesel_cli --version 2.0.0-rc.0 --no-default-features --features sqlite
fi
if [[ $(sd --version) != *"sd"* ]]; then
    echo "Installing sd..."
    cargo install sd
fi

diesel $@ --database-url "${ANYSPHEREROOT}"/client/daemon-rs/test.db

if [[ $@ == *"migration"* ]]; then
    echo "Replacing Timestamp with TimestamptzSqlite..."
    sd Timestamp TimestamptzSqlite "${ANYSPHEREROOT}"/client/daemon-rs/schema.rs
fi