#[macro_use]
extern crate diesel;
use diesel::prelude::*;
mod schema;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("daemon-rs/main.hpp");

        fn main_cc() -> i32;
    }
}

fn main() {
    ffi::main_cc();
}

use std::{error::Error, fmt};

#[derive(Debug)]
enum DbError<'a> {
    Ok(&'a str),
    Cancelled(&'a str),
    Unknown(&'a str),
    InvalidArgument(&'a str),
    DeadlineExceeded(&'a str),
    NotFound(&'a str),
    AlreadyExists(&'a str),
    PermissionDenied(&'a str),
    ResourceExhausted(&'a str),
    FailedPrecondition(&'a str),
    Aborted(&'a str),
    OutOfRange(&'a str),
    Unimplemented(&'a str),
    Internal(&'a str),
    Unavailable(&'a str),
    DataLoss(&'a str),
    Unauthenticated(&'a str),
}

impl Error for DbError<'_> {}

impl fmt::Display for DbError<'_> {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            DbError::Ok(s) => write!(f, "Ok: {}", s),
            DbError::Cancelled(s) => write!(f, "Cancelled: {}", s),
            DbError::Unknown(s) => write!(f, "Unknown: {}", s),
            DbError::InvalidArgument(s) => write!(f, "InvalidArgument: {}", s),
            DbError::DeadlineExceeded(s) => write!(f, "DeadlineExceeded: {}", s),
            DbError::NotFound(s) => write!(f, "NotFound: {}", s),
            DbError::AlreadyExists(s) => write!(f, "AlreadyExists: {}", s),
            DbError::PermissionDenied(s) => write!(f, "PermissionDenied: {}", s),
            DbError::ResourceExhausted(s) => write!(f, "ResourceExhausted: {}", s),
            DbError::FailedPrecondition(s) => write!(f, "FailedPrecondition: {}", s),
            DbError::Aborted(s) => write!(f, "Aborted: {}", s),
            DbError::OutOfRange(s) => write!(f, "OutOfRange: {}", s),
            DbError::Unimplemented(s) => write!(f, "Unimplemented: {}", s),
            DbError::Internal(s) => write!(f, "Internal: {}", s),
            DbError::Unavailable(s) => write!(f, "Unavailable: {}", s),
            DbError::DataLoss(s) => write!(f, "DataLoss: {}", s),
            DbError::Unauthenticated(s) => write!(f, "Unauthenticated: {}", s),
        }
    }
}

#[cxx::bridge(namespace = "db")]
pub mod db {

    #[derive(Queryable)]
    struct Friend {
        pub uid: i32,
        pub unique_name: String,
        pub display_name: String,
        pub enabled: bool,
        pub deleted: bool,
    }

    extern "Rust" {
        fn get_friend(uid: i32) -> Result<Friend>;
        fn get_friend_uid(uid: i32) -> i32;
    }

}

fn get_friend(uid: i32) -> Result<db::Friend, DbError<'static>> {
    let database_url = "/Users/arvid/code/anysphere/client/daemon-rs/test.db";
    let conn; 
    match SqliteConnection::establish(&database_url) {
        Ok(c) => conn = c,
        Err(e) => return Err(DbError::Unknown("failed to connect to database")),
    }

    use self::schema::friend;

    let mut v = friend::table.find(uid).load::<db::Friend>(&conn).expect("Error loading friend");
    if v.len() == 0 {
        return Err(DbError::NotFound("friend not found"));
    }
    Ok(v.remove(0))
}
fn get_friend_uid(uid: i32) -> i32 {
    51 + uid
}