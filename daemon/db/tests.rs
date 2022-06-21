use diesel::prelude::*;

use crate::db::*;
use rand::Rng;

fn get_registration_fragment() -> db::RegistrationFragment {
  let public_key: Vec<u8> = br#"zIUWz21AsWme9KxgS43TbrlaKYlLJqVMj/j1TKTIjx0="#.to_vec();
  let private_key: Vec<u8> = br#""EUSOOwjVEHRD1XzruR93LcK8YosZ3gWUkrrk8yjrpIQ="#.to_vec();
  let allocation: i32 = 34;
  let pir_secret_key: Vec<u8> = br#""hi hi"#.to_vec();
  let pir_galois_key: Vec<u8> = br#""hi hi hi"#.to_vec();
  let authentication_token: String = "X6H3ILWIrDGThjbi4IpYfWGtJ3YWdMIf".to_string();

  db::RegistrationFragment {
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
  let db = init(db_file.as_str()).unwrap();

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

#[test]
fn test_receive_msg() {
  let db_file = gen_temp_file();
  let db = init(db_file.as_str()).unwrap();

  let config_data = get_registration_fragment();
  db.do_register(config_data).unwrap();

  let f = db.create_friend("friend_1", "Friend 1", 20).unwrap();
  db.add_friend_address(
    db::AddAddress {
      unique_name: "friend_1".to_string(),
      read_index: 0,
      read_key: br#"xxxx"#.to_vec(),
      write_key: br#"xxxx"#.to_vec(),
    },
    20,
  )
  .unwrap();

  let msg = "hi im a chunk";
  let did_get_full_msg = db
    .receive_chunk(
      db::IncomingChunkFragment {
        from_friend: f.uid,
        sequence_number: 1,
        chunks_start_sequence_number: 1,
        content: "hi im a chunk".to_string(),
      },
      1,
    )
    .unwrap();

  assert!(did_get_full_msg);

  let msgs = db
    .get_received_messages(db::MessageQuery {
      limit: -1,
      filter: db::MessageFilter::All,
      delivery_status: db::DeliveryStatus::Delivered,
      sort_by: db::SortBy::None,
      after: 0,
    })
    .unwrap();

  assert_eq!(msgs.len(), 1);
  assert_eq!(msgs[0].content, msg);

  let mut conn = SqliteConnection::establish(&db.address).unwrap();
  use crate::schema::status;
  let status_pair = status::table
    .find(msgs[0].uid)
    .select((status::sent_acked_seqnum, status::received_seqnum))
    .first::<(i32, i32)>(&mut conn)
    .unwrap();
  assert_eq!(status_pair.0, 0);
  assert_eq!(status_pair.1, 1);
}

#[test]
fn test_send_msg() {
  let db_file = gen_temp_file();
  let db = init(db_file.as_str()).unwrap();

  let config_data = get_registration_fragment();
  db.do_register(config_data).unwrap();

  let f = db.create_friend("friend_1", "Friend 1", 20).unwrap();
  db.add_friend_address(
    db::AddAddress {
      unique_name: "friend_1".to_string(),
      read_index: 0,
      read_key: br#"xxxx"#.to_vec(),
      write_key: br#"wwww"#.to_vec(),
    },
    20,
  )
  .unwrap();

  let msg = "hi im a single chunk";
  db.queue_message_to_send("friend_1", msg, vec![msg.to_string()]).unwrap();

  let chunk_to_send = db.chunk_to_send(vec![]).unwrap();

  assert!(chunk_to_send.to_friend == f.uid);
  assert!(chunk_to_send.sequence_number == 1);
  assert!(chunk_to_send.chunks_start_sequence_number == 1);
  // assert!(chunk_to_send.message_uid == 0); // we don't necessarily know what message_uid sqlite chooses
  assert!(chunk_to_send.content == msg);
  assert!(chunk_to_send.write_key == br#"wwww"#.to_vec());
  assert!(chunk_to_send.num_chunks == 1);
}
