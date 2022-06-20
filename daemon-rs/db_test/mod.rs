pub use crate::db;

#[cfg(test)]
mod registration_tests {

  use crate::db::db::*;
  use crate::db::DB;
  use rand::Rng;

  fn get_registration_fragment() -> RegistrationFragment {
    let public_key: Vec<u8> = br#"zIUWz21AsWme9KxgS43TbrlaKYlLJqVMj/j1TKTIjx0="#.to_vec();
    let private_key: Vec<u8> = br#""EUSOOwjVEHRD1XzruR93LcK8YosZ3gWUkrrk8yjrpIQ="#.to_vec();
    let allocation: i32 = 34;
    let pir_secret_key: Vec<u8> = br#""hi hi"#.to_vec();
    let pir_galois_key: Vec<u8> = br#""hi hi hi"#.to_vec();
    let authentication_token: String = "X6H3ILWIrDGThjbi4IpYfWGtJ3YWdMIf".to_string();

    RegistrationFragment {
      public_key,
      private_key,
      allocation,
      pir_secret_key,
      pir_galois_key,
      authentication_token,
    }
  }

  fn gen_temp_file() -> String {
    // generate a temporary file in the current path that ends with .db using std library
    let mut rng = rand::thread_rng();

    let random_temp_string: String =
      (&mut rng).sample_iter(rand::distributions::Alphanumeric).take(7).map(char::from).collect();
    // get the current path
    let current_path = std::env::current_dir().unwrap();
    let temp_file_name = format!("{}-{}.db", current_path.display(), random_temp_string);
    temp_file_name
  }

  #[test]
  fn test_connection() {
    // TODO: fix this to be generic like a real test lol
    let db_file = gen_temp_file();
    println!("db_file: {}", db_file);
    let db = DB { address: db_file.to_string() };

    let connection = db.connect();
    assert!(connection.is_ok());
  }

  #[test]
  fn test_register() {
    let config_data = get_registration_fragment();
    let config_clone = get_registration_fragment();

    let db_file = gen_temp_file();
    println!("db_file: {}", db_file);
    let db = crate::db::init(db_file.as_str()).unwrap();

    db.delete_registration().unwrap();
    assert!(db.has_registered().unwrap() == false);

    let did_register = db.do_register(config_data);

    unsafe {
      db.dump().unwrap();
    }

    match did_register {
      Ok(_) => {
        assert!(db.has_registered().unwrap() == true);
      }
      Err(e) => {
        panic!("{:?}", e);
      }
    }

    let new_registration = db.get_registration();

    match new_registration {
      Ok(registration) => {
        assert_eq!(registration.public_key, config_clone.public_key);
        assert_eq!(registration.private_key, config_clone.private_key);
        assert_eq!(registration.allocation, config_clone.allocation);
        assert_eq!(registration.pir_secret_key, config_clone.pir_secret_key);
        assert_eq!(registration.pir_galois_key, config_clone.pir_galois_key);
        assert_eq!(registration.authentication_token, config_clone.authentication_token);
      }
      Err(_) => {
        assert!(false);
      }
    }

    db.delete_registration().unwrap();

    assert!(db.has_registered().unwrap() == false);
  }
}
