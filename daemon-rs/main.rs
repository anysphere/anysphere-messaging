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
        fn get_friend(uid: i32) -> Friend;
        fn get_friend_uid(uid: i32) -> i32;
    }
}

fn get_friend(uid: i32) -> db::Friend {
    let database_url = "/Users/arvid/code/anysphere/client/daemon-rs/test.db";
    let conn = SqliteConnection::establish(&database_url)
        .unwrap_or_else(|_| panic!("Error connecting to {}", database_url));

    use self::schema::friend;

    let mut v = friend::table.find(uid).load::<db::Friend>(&conn).expect("Error loading friend");
    if v.len() == 0 {
        panic!("No friend with uid {}", uid);
    }
    v.remove(0)
}
fn get_friend_uid(uid: i32) -> i32 {
    51 + uid
}