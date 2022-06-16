// Copyright 2015 The Bazel Authors. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#[macro_use]
extern crate diesel;

pub mod models;
pub mod schema;

mod greeter;

use diesel::prelude::*;
use diesel::insert_into;
use models::Friend;

#[cxx::bridge]
mod ffi {

    extern "Rust" {
        fn f() -> i32;
    }

    unsafe extern "C++" {
        include!("daemon-rs/src/test.hpp");

        fn test_cpp() -> i32;
    }
}

fn f() -> i32 {
    return 69;
}

fn main() {
    let hello = greeter::Greeter::new("Hello");
    hello.greet("world");

    let i = ffi::test_cpp();
    println!("Test_cpp returns {}", i);

    let database_url = "/Users/arvid/code/anysphere/client/test.db";
    let conn = SqliteConnection::establish(&database_url)
        .unwrap_or_else(|_| panic!("Error connecting to {}", database_url));
    
    use self::schema::friends::dsl::*;
    
    insert_into(friends).values((unique_name.eq("arvid"), display_name.eq("Arvid"))).execute(&conn).expect("Error inserting arvid.");

    let results = friends.filter(enabled.eq(true)).limit(5).load::<Friend>(&conn).expect("Error loading friends");

    println!("Displaying {} posts", results.len());
    for friend in results {
        println!("{}", friend.unique_name);
        println!("----------\n");
        println!("{}", friend.display_name);
    }
}
