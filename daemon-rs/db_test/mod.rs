pub use crate::db;

#[cfg(test)]
mod registration_tests {

  #[test]
  fn test_connection() {
    // TODO: fix this to be generic like a real test lol
    let db = crate::db::DB {
      address: "/Users/sualeh/code/anysphere/client/daemon-rs/test.db".to_string(),
    };

    let connection = db.connect();
    assert!(connection.is_ok());
  }



}