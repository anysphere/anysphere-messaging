#[macro_use]
extern crate diesel;

extern crate libsqlite3_sys;

pub mod db;

pub mod schema;

#[cfg(test)]
mod tests;