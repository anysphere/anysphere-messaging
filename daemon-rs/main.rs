#[macro_use]
extern crate diesel;

mod db;

fn main() {
    use std::env;
    let args: Vec<String> = env::args().collect();
    ffi::main_cc(args);
}

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("daemon-rs/main.hpp");

        fn main_cc(args: Vec<String>) -> i32;
    }
}