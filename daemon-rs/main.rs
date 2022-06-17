#[macro_use]
extern crate diesel;
use diesel::prelude::*;
mod schema;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("daemon-rs/main.hpp");

        fn main_cc(args: Vec<String>) -> i32;
    }
}

fn main() {
    use std::env;
    let args: Vec<String> = env::args().collect();
    ffi::main_cc(args);
}

use std::{error::Error, fmt};

#[derive(Debug)]
enum DbError {
    Ok(String),
    Cancelled(String),
    Unknown(String),
    InvalidArgument(String),
    DeadlineExceeded(String),
    NotFound(String),
    AlreadyExists(String),
    PermissionDenied(String),
    ResourceExhausted(String),
    FailedPrecondition(String),
    Aborted(String),
    OutOfRange(String),
    Unimplemented(String),
    Internal(String),
    Unavailable(String),
    DataLoss(String),
    Unauthenticated(String),
}

impl Error for DbError {}

impl fmt::Display for DbError {
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

// TODO(arvid): manage a connection pool for the DB here?
// i.e. pool: Box<ConnectionPool> or something
struct DB {
    address: String,
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

    #[derive(Queryable)]
    struct Address {
        pub uid: i32,
        pub read_index: i32,
        pub ack_index: i32,
        pub read_key: Vec<u8>,
        pub write_key: Vec<u8>,
    }

    #[derive(Queryable)]
    struct Registration {
        pub uid: i32,
        pub public_key: Vec<u8>,
        pub private_key: Vec<u8>,
        pub allocation: i32,
        pub pir_secret_key: Vec<u8>,
        pub pir_galois_key: Vec<u8>,
        pub authentication_token: String,
    }

    extern "Rust" {
        type DB;
        fn init(address: &str, default_server_address: &str, default_latency: i32) -> Result<Box<DB>>;

        fn has_registered(&self) -> bool;
        fn get_registration(&self) -> Result<Registration>;
        fn get_pir_secret_key(&self) -> Result<Vec<u8>>;

        fn set_latency(&self, latency: i32, default_latency: i32) -> Result<()>;
        fn get_latency(&self) -> Result<i32>;
        fn set_server_address(&self, server_address: &str, default_server_address: &str) -> Result<()>;
        fn get_server_address(&self) -> Result<String>;

        fn get_friend(&self, uid: i32) -> Result<Friend>;
    }

}

fn init(address: &str, default_server_address: &str, default_latency: i32) -> Result<Box<DB>, DbError> {
    let db = DB {
        address: address.to_string(),
    };
    let conn = db.connect()?;

    // TODO(arvid): run migrations

    use self::schema::config;

    let q = diesel::update(config::table.filter(config::server_address_default.eq(true))).set(config::server_address.eq(default_server_address));
    match q.execute(&conn) {
        Ok(_) => (),
        Err(e) => return Err(DbError::Unknown(format!("failed to set default server address: {}", e))),
    };

    let q = diesel::update(config::table.filter(config::latency_default.eq(true))).set(config::latency.eq(default_latency));
    match q.execute(&conn) {
        Ok(_) => (),
        Err(e) => return Err(DbError::Unknown(format!("failed to set default latency: {}", e))),
    };

    Ok(Box::new(db))
}

fn print_query<T: diesel::query_builder::QueryFragment<diesel::sqlite::Sqlite>>(q: &T) {
    println!("print_query: {}", diesel::debug_query::<diesel::sqlite::Sqlite, _>(&q));
}

impl DB {
    fn connect(&self) -> Result<SqliteConnection, DbError> {
        match SqliteConnection::establish(&self.address) {
            Ok(c) => return Ok(c),
            Err(e) => return Err(DbError::Unknown("failed to connect to database".to_string())),
        }
    }

    fn has_registered(&self) -> Result<bool, DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        let q = config::table.select(config::has_registered);
        let has_registered = q.first(&conn).map_err(|e| DbError::Unknown(format!("failed to query has_registered: {}", e)))?;
        Ok(has_registered)
    }

    fn get_registration(&self) -> Result<Registration, DbError> {
        let conn = self.connect()?;
        use self::schema::registration;

        let q = registration::table.select(registration::all_columns);
        let registration = q.first(&conn).map_err(|e| DbError::Unknown(format!("failed to query registration: {}", e)))?;
        Ok(registration)
    }

    fn get_pir_secret_key(&self) -> Result<Vec<u8>> {
        let conn = self.connect()?;
        use self::schema::registration;

        let q = registration::table.select(registration::pir_secret_key);
        let pir_secret_key = q.first(&conn).map_err(|e| DbError::Unknown(format!("failed to query pir_secret_key: {}", e)))?;
        Ok(pir_secret_key)
    }

    fn get_friend(&self, uid: i32) -> Result<db::Friend, DbError> {
        let conn = self.connect()?;
        use self::schema::friend;

        if let Ok(mut v) = friend::table.find(uid).load::<db::Friend>(&conn) {
            if v.len() == 0 {
                return Err(DbError::NotFound("friend not found".to_string()));
            }
            Ok(v.remove(0))
        } else {
            Err(DbError::Unknown("failed to get friend".to_string()))
        }
    }


    fn set_latency(&self, latency: i32, default_latency: i32) -> Result<(), DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        let r = conn.transaction::<_, diesel::result::Error, _>(|| {
            if latency == default_latency {
                diesel::update(config::table).set(config::latency_default.eq(true)).execute(&conn)?;
            } else {
                diesel::update(config::table).set(config::latency_default.eq(true)).execute(&conn)?;
            }
            diesel::update(config::table).set(config::latency.eq(latency)).execute(&conn)
        });

        match r {
            Ok(_) => Ok(()),
            Err(e) => Err(DbError::Unknown(format!("set_latency: {}", e))),
        }
    }

    fn get_latency(&self) -> Result<i32, DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        let q = config::table.select(config::latency);
        let latency = q.first(&conn).map_err(|e| DbError::Unknown(format!("failed to query latency: {}", e)))?;
        Ok(latency)
    }

    fn set_server_address(&self, server_address: &str, default_server_address: &str) -> Result<(), DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        let r = conn.transaction::<_, diesel::result::Error, _>(|| {
            if server_address == default_server_address {
                diesel::update(config::table).set(config::server_address_default.eq(true)).execute(&conn)?;
            } else {
                diesel::update(config::table).set(config::server_address_default.eq(false)).execute(&conn)?;
            }
            diesel::update(config::table).set(config::server_address.eq(server_address)).execute(&conn)
        });

        match r {
            Ok(_) => Ok(()),
            Err(e) => Err(DbError::Unknown(format!("set_server_address: {}", e))),
        }
    }

    fn get_server_address(&self) -> Result<String, DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        match config::table.select(config::server_address).first(&conn) {
            Ok(s) => match s {
                Some(s) => Ok(s),
                None => Err(DbError::NotFound("server address not found".to_string())),
            },
            Err(e) => Err(DbError::Unknown(format!("get_server_address: {}", e))),
        }
    }

}