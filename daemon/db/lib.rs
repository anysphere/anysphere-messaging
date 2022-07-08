//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

#[macro_use]
extern crate diesel;

extern crate libsqlite3_sys;

extern crate anyhow;

pub mod db;

pub mod schema;

#[cfg(test)]
mod tests;
