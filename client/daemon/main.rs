mod cli;

fn main() {
  use std::env;
  let args: Vec<String> = env::args().collect();

  let result = cli::cli(args);
  ffi::main_cc(result);
  // we can never get here. main_cc will always exit the process.
  panic!("main_cc should never return");
}

#[cxx::bridge]
mod ffi {
  unsafe extern "C++" {
    include!("client/daemon/main.hpp");
    fn main_cc(args: Vec<String>);
  }
}
