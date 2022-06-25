use diesel::prelude::*;

use crate::{db::*};
use rand::{random, Rng};

fn get_registration_fragment() -> ffi::RegistrationFragment {
  let kx_public_key: Vec<u8> = br#"zIUWz21AsWme9KxgS43TbrlaKYlLJqVMj/j1TKTIjx0="#.to_vec();
  let kx_private_key: Vec<u8> = br#""EUSOOwjVEHRD1XzruR93LcK8YosZ3gWUkrrk8yjrpIQ="#.to_vec();
  let friend_request_public_key: Vec<u8> = br#"zIUWz21AsWme9KxgS56TbrlaKYlLJqVMj/j1TKTIjx0="#.to_vec();
  let friend_request_private_key: Vec<u8> = br#"zIUWz21AsWme9KxgS78TbrlaKYlLJqVMj/j1TKTIjx0="#.to_vec();
  let allocation: i32 = 34;
  let pir_secret_key: Vec<u8> = br#""hi hi"#.to_vec();
  let pir_galois_key: Vec<u8> = br#""hi hi hi"#.to_vec();
  let authentication_token: String = "X6H3ILWIrDGThjbi4IpYfWGtJ3YWdMIf".to_string();
  let public_id: String = "wwww".to_string();

  ffi::RegistrationFragment {
    friend_request_public_key,
    friend_request_private_key,
    kx_public_key,
    kx_private_key,
    allocation,
    pir_secret_key,
    pir_galois_key,
    authentication_token,
    public_id
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
  let db = DB { address: db_file };

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
  assert!(!db.has_registered().unwrap());

  let did_register = db.do_register(config_data);

  unsafe {
    db.dump().unwrap();
  }

  match did_register {
    Ok(_) => {
      assert!(db.has_registered().unwrap());
    }
    Err(e) => {
      panic!("{:?}", e);
    }
  }

  let new_registration = db.get_registration();

  match new_registration {
    Ok(registration) => {
      assert_eq!(registration.kx_public_key, config_clone.kx_public_key);
      assert_eq!(registration.kx_private_key, config_clone.kx_private_key);
      assert_eq!(registration.friend_request_public_key, config_clone.friend_request_public_key);
      assert_eq!(registration.friend_request_private_key, config_clone.friend_request_private_key);
      assert_eq!(registration.allocation, config_clone.allocation);
      assert_eq!(registration.pir_secret_key, config_clone.pir_secret_key);
      assert_eq!(registration.pir_galois_key, config_clone.pir_galois_key);
      assert_eq!(registration.authentication_token, config_clone.authentication_token);
      assert_eq!(registration.public_id, config_clone.public_id);
    }
    Err(_) => {
      panic!("Failed to get registration");
    }
  }

  db.delete_registration().unwrap();

  assert!(!db.has_registered().unwrap());
}

#[test]
fn test_receive_msg() {
  let db_file = gen_temp_file();
  let db = init(db_file.as_str()).unwrap();

  let config_data = get_registration_fragment();
  db.do_register(config_data).unwrap();

  let f = db.create_friend("friend_1", "Friend 1", "tttt", 20).unwrap();
  db.add_friend_address(
    ffi::AddAddress {
      unique_name: "friend_1".to_string(),
      kx_public_key: br#"uuuu"#.to_vec(),
      friend_request_public_key: br#"vvvv"#.to_vec(),
      friend_request_message: "hello".to_string(),
      read_index: 0,
      read_key: br#"xxxx"#.to_vec(),
      write_key: br#"wwww"#.to_vec(),
    },
    20,
  )
  .unwrap();

  let msg = "hi im a chunk";
  let chunk_status = db
    .receive_chunk(
      ffi::IncomingChunkFragment {
        from_friend: f.uid,
        sequence_number: 1,
        chunks_start_sequence_number: 1,
        content: "hi im a chunk".to_string(),
      },
      1,
    )
    .unwrap();

  assert!(chunk_status == ffi::ReceiveChunkStatus::NewChunkAndNewMessage);

  let msgs = db
    .get_received_messages(ffi::MessageQuery {
      limit: -1,
      filter: ffi::MessageFilter::All,
      delivery_status: ffi::DeliveryStatus::Delivered,
      sort_by: ffi::SortBy::None,
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

  let acks_to_send = db.acks_to_send().unwrap();
  assert_eq!(acks_to_send.len(), 1);
  assert_eq!(acks_to_send[0].to_friend, f.uid);
  assert_eq!(acks_to_send[0].ack, 1);
  assert_eq!(acks_to_send[0].write_key, br#"wwww"#.to_vec());
  // assert_eq!(acks_to_send[0].ack_index, ?); // not sure what this should be
}

#[test]
fn test_send_msg() {
  let db_file = gen_temp_file();
  let db = init(db_file.as_str()).unwrap();

  let config_data = get_registration_fragment();
  db.do_register(config_data).unwrap();

  let f = db.create_friend("friend_1", "Friend 1", "tttt", 20).unwrap();
  db.add_friend_address(
    ffi::AddAddress {
      unique_name: "friend_1".to_string(),
      kx_public_key: br#"uuuu"#.to_vec(),
      friend_request_public_key: br#"vvvv"#.to_vec(),
      friend_request_message: "hello".to_string(),
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

// Simulate adding a friend
// Plot: we receive an incoming async friend request, and approve it
#[test]
fn test_async_add_friend() {
  let db_file = gen_temp_file();
  let db = init(db_file.as_str()).unwrap();

  let config_data = get_registration_fragment();
  db.do_register(config_data).unwrap();

  // check initial state
  assert!(db.has_space_for_async_friend_requests().unwrap());
  assert!(db.get_incoming_async_friend_requests().unwrap().is_empty());
  // add an incoming friend request
  let friend_name = "friend_1";
  let friend_request = ffi::FriendFragment {
    unique_name: friend_name.to_string(),
    display_name: "lyrica".to_string(),
    public_id: "tttt".to_string(),
    progress: INCOMING_REQUEST,
    deleted: false,
  };

  let address = ffi::AddAddress {
    unique_name: friend_name.to_string(),
    read_index: 0,
    read_key: br#"xxxx"#.to_vec(),
    write_key: br#"wwww"#.to_vec(),
    kx_public_key: br#"xxxx"#.to_vec(),
    friend_request_public_key: br#"xxxx"#.to_vec(),
    friend_request_message: "finally made a friend".to_string(),
  };
  db.add_incoming_async_friend_requests(friend_request, address).unwrap();
  let friend_requests = db.get_incoming_async_friend_requests().unwrap();
  // check that we have a friend request
  assert_eq!(friend_requests.len(), 1);
  assert_eq!(friend_requests[0].unique_name, friend_name);
  assert_eq!(friend_requests[0].public_id, "tttt");

  // this uid now identifies the friend
  let uid = friend_requests[0].uid;
  let address = db.get_friend_address(uid).unwrap();
  // check the associated address & status struct
  // the ack index shouldn't have been set yet
  assert!(address.ack_index < 0);
  assert!(address.uid == uid);

  // approve the friend request
  let max_friend = 99;
  db.approve_async_friend_request(friend_name, max_friend).unwrap();
  // check that the friend request is gone
  let friend_requests_new = db.get_incoming_async_friend_requests().unwrap();
  assert_eq!(friend_requests_new.len(), 0);
  // check that we have a friend
  let friends = db.get_friends().unwrap();
  assert_eq!(friends.len(), 1);
  assert_eq!(friends[0].uid, uid);
  assert_eq!(friends[0].public_id, "tttt");
  assert_eq!(friends[0].unique_name, "friend_1");
  // check the friend address
  let new_address = db.get_friend_address(uid).unwrap();
  assert_eq!(new_address.uid, uid);
  assert!(new_address.ack_index >= 0);
}
