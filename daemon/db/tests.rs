//
// Copyright 2022 Anysphere, Inc.
// SPDX-License-Identifier: GPL-3.0-only
//

use diesel::prelude::*;

use crate::db::*;
use rand::Rng;

/**
 * Test the database.
 *
 * Partition tested:
 * - 2 friends
 * - async addition of friends
 *
 * - test_recieive_msg(): basic functionality of the following APIs:
 *  - add_outgoing_async_invitation
 *  - add_incoming_async_invitation
 *  - get_received_messages
 *    - 1 message sent
 *  - get_sent_messages
 *   - 1 message sent
 *  - acks_to_send()
 *  
 *  - test_send_msg(): basic functionality of the following APIs:
 *    - add_outgoing_async_invitation
 *    - add_incoming_async_invitation
 *    - queue_message_to_send
 *    - chunk_to_send()
 *    - do_register()
 */

fn get_registration_fragment() -> ffi::RegistrationFragment {
  let kx_public_key: Vec<u8> = br#"zIUWz21AsWme9KxgS43TbrlaKYlLJqVMj/j1TKTIjx0="#.to_vec();
  let kx_private_key: Vec<u8> = br#""EUSOOwjVEHRD1XzruR93LcK8YosZ3gWUkrrk8yjrpIQ="#.to_vec();
  let invitation_public_key: Vec<u8> = br#"zIUWz21AsWme9KxgS56TbrlaKYlLJqVMj/j1TKTIjx0="#.to_vec();
  let invitation_private_key: Vec<u8> = br#"zIUWz21AsWme9KxgS78TbrlaKYlLJqVMj/j1TKTIjx0="#.to_vec();
  let allocation: i32 = 34;
  let pir_secret_key: Vec<u8> = br#""hi hi"#.to_vec();
  let pir_galois_key: Vec<u8> = br#""hi hi hi"#.to_vec();
  let authentication_token: String = "X6H3ILWIrDGThjbi4IpYfWGtJ3YWdMIf".to_string();
  let public_id: String = "my_public_id".to_string();

  ffi::RegistrationFragment {
    invitation_public_key,
    invitation_private_key,
    kx_public_key,
    kx_private_key,
    allocation,
    pir_secret_key,
    pir_galois_key,
    authentication_token,
    public_id,
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
      assert_eq!(registration.invitation_public_key, config_clone.invitation_public_key);
      assert_eq!(registration.invitation_private_key, config_clone.invitation_private_key);
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

  // add friend by issuing an outgoing invitation and then accepting an incoming invitation

  let params = ffi::AddOutgoingAsyncInvitationParams {
    unique_name: "friend_1".to_string(),
    display_name: "Friend 1".to_string(),
    public_id: "hi_this_is_a_public_id".to_string(),
    invitation_public_key: br#"fffff"#.to_vec(),
    kx_public_key: br#"kxkxkx"#.to_vec(),
    message: "message hi hi".to_string(),
    read_index: 0,
    read_key: br#"rrrrr"#.to_vec(),
    write_key: br#"wwww"#.to_vec(),
    max_friends: 20,
  };

  let f = db.add_outgoing_async_invitation(params).unwrap();
  // will be auto accepted!
  db.add_incoming_async_invitation("hi_this_is_a_public_id", "invitation: hi from friend 1")
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

  // 1 invitation message + 1 actual message
  assert_eq!(msgs.len(), 2);
  assert_eq!(msgs[0].content, "invitation: hi from friend 1");
  assert_eq!(msgs[1].content, msg);

  let mut conn = SqliteConnection::establish(&db.address).unwrap();
  use crate::schema::transmission;
  let status_pair = transmission::table
    .find(f.uid)
    .select((transmission::sent_acked_seqnum, transmission::received_seqnum))
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

  let params = ffi::AddOutgoingAsyncInvitationParams {
    unique_name: "friend_1".to_string(),
    display_name: "Friend 1".to_string(),
    public_id: "hi_this_is_a_public_id".to_string(),
    invitation_public_key: br#"fffff"#.to_vec(),
    kx_public_key: br#"kxkxkx"#.to_vec(),
    message: "message hi hi".to_string(),
    read_index: 0,
    read_key: br#"rrrrr"#.to_vec(),
    write_key: br#"wwww"#.to_vec(),
    max_friends: 20,
  };

  let f = db.add_outgoing_async_invitation(params).unwrap();

  // copy the above call

  println!("f: {:?}", f);

  // will be auto accepted!
  db.add_incoming_async_invitation("hi_this_is_a_public_id", "hi from freidn 1").unwrap();

  let msg = "hi im a single chunk";
  db.queue_message_to_send(vec!["friend_1".to_string()], msg, vec![msg.to_string()]).unwrap();

  let chunk_to_send = db.chunk_to_send(vec![]).unwrap();

  // the chunk to send will be the system message for the outgoing invitation
  // the content is the public id
  assert!(chunk_to_send.to_friend == f.uid);
  assert!(chunk_to_send.sequence_number == 1);
  assert!(chunk_to_send.chunks_start_sequence_number == 1);
  // assert!(chunk_to_send.message_uid == 0); // we don't necessarily know what message_uid sqlite chooses
  assert!(chunk_to_send.content == "my_public_id");
  assert!(chunk_to_send.write_key == br#"wwww"#.to_vec());
  assert!(chunk_to_send.num_chunks == 1);

  db.receive_ack(f.uid, 1).unwrap();

  let chunk_to_send = db.chunk_to_send(vec![]).unwrap();

  // the chunk to send will be the system message for the outgoing invitation
  // the content is the public id
  assert!(chunk_to_send.to_friend == f.uid);
  assert!(chunk_to_send.sequence_number == 2);
  assert!(chunk_to_send.chunks_start_sequence_number == 2);
  // assert!(chunk_to_send.message_uid == 0); // we don't necessarily know what message_uid sqlite chooses
  assert!(chunk_to_send.content == "hi im a single chunk");
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
  assert!(db.has_space_for_async_invitations().unwrap());
  assert!(db.get_incoming_invitations().unwrap().is_empty());
  // add an incoming friend request
  let friend_name = "friend_1";

  db.add_incoming_async_invitation("fake_public_id_string", "hi! do you want to be my friend?")
    .unwrap();
  let invitations = db.get_incoming_invitations().unwrap();
  // check that we have a friend request
  assert_eq!(invitations.len(), 1);
  assert_eq!(invitations[0].public_id, "fake_public_id_string");
  assert_eq!(invitations[0].message, "hi! do you want to be my friend?");

  // approve the friend request
  let max_friends = 20;

  // `ffi::AcceptIncomingInvitationParams { public_id: val, unique_name: val, display_name: val, invitation_public_key: val, kx_public_key: val, read_index: val, read_key: val, write_key: val, max_friends: val }`

  let params = ffi::AcceptIncomingInvitationParams {
    public_id: "fake_public_id_string".to_string(),
    unique_name: friend_name.to_string(),
    display_name: "Display Name".to_string(),
    invitation_public_key: br#"xPubxxx"#.to_vec(),
    kx_public_key: br#"xKxxxx"#.to_vec(),
    read_index: 0,
    read_key: br#"rrrrrrrr"#.to_vec(),
    write_key: br#"wwww"#.to_vec(),
    max_friends,
  };

  db.accept_incoming_invitation(params).unwrap();

  // check that the friend request is gone
  let invitations_new = db.get_incoming_invitations().unwrap();
  assert_eq!(invitations_new.len(), 0);
  // check that we have a friend
  let friends = db.get_friends().unwrap();
  assert_eq!(friends.len(), 1);
  assert_eq!(friends[0].public_id, "fake_public_id_string");
  assert_eq!(friends[0].unique_name, friend_name);
  // check the friend address
  let new_address = db.get_friend_address(friends[0].uid).unwrap();
  assert_eq!(new_address.uid, friends[0].uid);
  assert!(new_address.ack_index >= 0);
}
