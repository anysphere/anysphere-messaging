#[macro_use]
extern crate diesel;

extern crate db;

#[test]
fn test_greeting() {
  // let hello = greeter::Greeter::new("Hello");
  assert_eq!("Hello world", "Hello world");
}