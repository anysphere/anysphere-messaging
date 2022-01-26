#!/usr/bin/env bash

PGPASSWORD="$2" psql -U postgres -d postgres -h "$1" -f schema.sql
./as_server -d "$1" -p "$2"