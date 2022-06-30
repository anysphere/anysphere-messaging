use diesel::prelude::*;
use std::collections::HashMap;

use std::{error::Error, fmt};

use anyhow::Context;

#[derive(Debug)]
#[allow(dead_code)]
pub enum DbError {
  Ok(String),
  Cancelled(String),
  Unknown(String),
  InvalidArgument(String),
  DeadlineExceeded(String),
  NotFound(String),
  AlreadyExists(String),
  PermissionDenied(String),
  ResourceExhausted(String),
  FailedPrecondition(String),
  Aborted(String),
  OutOfRange(String),
  Unimplemented(String),
  Internal(String),
  Unavailable(String),
  DataLoss(String),
  Unauthenticated(String),
}

// this is only used in check_rep()
#[cfg(debug_assertions)]
#[derive(Queryable, Debug)]
struct ChunkInterestingNumbers {
  to_friend: i32,
  chunks_start_sequence_number: i32,
  message_uid: Option<i32>,
}
#[cfg(debug_assertions)]
#[derive(Queryable, Debug)]
struct IncomingChunkInterestingNumbers {
  from_friend: i32,
  chunks_start_sequence_number: i32,
  message_uid: i32,
}

#[derive(Queryable)]
struct OutgoingChunkPlusPlusMinusNumChunks {
  pub to_friend: i32,
  pub sequence_number: i32,
  pub chunks_start_sequence_number: i32,
  #[diesel(deserialize_as = crate::db::I32ButMinusOneIsNone)]
  pub message_uid: i32, // -1 iff system message
  pub content: String,
  pub write_key: Vec<u8>,
  pub system: bool,
  #[diesel(deserialize_as = ffi::SystemMessage)]
  pub system_message: ffi::SystemMessage,
}

impl Error for DbError {}

impl fmt::Display for DbError {
  fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
    match self {
      DbError::Ok(s) => write!(f, "Ok: {}", s),
      DbError::Cancelled(s) => write!(f, "Cancelled: {}", s),
      DbError::Unknown(s) => write!(f, "Unknown: {}", s),
      DbError::InvalidArgument(s) => write!(f, "InvalidArgument: {}", s),
      DbError::DeadlineExceeded(s) => write!(f, "DeadlineExceeded: {}", s),
      DbError::NotFound(s) => write!(f, "NotFound: {}", s),
      DbError::AlreadyExists(s) => write!(f, "AlreadyExists: {}", s),
      DbError::PermissionDenied(s) => write!(f, "PermissionDenied: {}", s),
      DbError::ResourceExhausted(s) => write!(f, "ResourceExhausted: {}", s),
      DbError::FailedPrecondition(s) => write!(f, "FailedPrecondition: {}", s),
      DbError::Aborted(s) => write!(f, "Aborted: {}", s),
      DbError::OutOfRange(s) => write!(f, "OutOfRange: {}", s),
      DbError::Unimplemented(s) => write!(f, "Unimplemented: {}", s),
      DbError::Internal(s) => write!(f, "Internal: {}", s),
      DbError::Unavailable(s) => write!(f, "Unavailable: {}", s),
      DbError::DataLoss(s) => write!(f, "DataLoss: {}", s),
      DbError::Unauthenticated(s) => write!(f, "Unauthenticated: {}", s),
    }
  }
}

// keep this in sync with message.proto

impl<DB> Queryable<diesel::sql_types::Integer, DB> for ffi::SystemMessage
where
  DB: diesel::backend::Backend,
  i32: diesel::deserialize::FromSql<diesel::sql_types::Integer, DB>,
{
  type Row = i32;
  fn build(s: i32) -> diesel::deserialize::Result<Self> {
    match s {
      0 => Ok(ffi::SystemMessage::OutgoingInvitation),
      _ => Err("invalid system message".into()),
    }
  }
}

impl ToSql<diesel::sql_types::Integer, diesel::sqlite::Sqlite> for ffi::SystemMessage
where
  i32: ToSql<diesel::sql_types::Integer, diesel::sqlite::Sqlite>,
{
  fn to_sql<'b>(
    &'b self,
    out: &mut Output<'b, '_, diesel::sqlite::Sqlite>,
  ) -> diesel::serialize::Result {
    match *self {
      ffi::SystemMessage::OutgoingInvitation => out.set_value(0 as i32),
      _ => return Err("invalid system message".into()),
    }
    Ok(diesel::serialize::IsNull::No)
  }
}

// TODO(arvid): manage a connection pool for the DB here?
// i.e. pool: Box<ConnectionPool> or something
pub struct DB {
  pub address: String,
}

#[cxx::bridge]
mod util {
  unsafe extern "C++" {
    // we use absl::Time because it is SUCH a well-thought out library
    // we trust it more than chrono or rust's time
    include!("daemon/util.hpp");
    fn unix_micros_now() -> i64;
  }
}

#[derive(Insertable)]
#[diesel(table_name = crate::schema::sent)]
struct Sent {
  pub uid: i32,
  pub to_friend: i32,
  pub num_chunks: i32,
  pub sent_at: i64, // unix micros
  pub delivered: bool,
  pub delivered_at: Option<i64>, // unix micros
}

#[derive(Insertable)]
#[diesel(table_name = crate::schema::received)]
struct Received {
  pub uid: i32,
  pub from_friend: i32,
  pub num_chunks: i32,
  pub received_at: i64, // unix micros
  pub delivered: bool,
  pub delivered_at: Option<i64>, // unix micros
  pub seen: bool,
}

pub struct I64ButZeroIsNone(i64);

impl From<I64ButZeroIsNone> for i64 {
  fn from(x: I64ButZeroIsNone) -> i64 {
    x.0
  }
}

impl<DB> Queryable<diesel::sql_types::Nullable<diesel::sql_types::BigInt>, DB> for I64ButZeroIsNone
where
  DB: diesel::backend::Backend,
  Option<i64>:
    diesel::deserialize::FromSql<diesel::sql_types::Nullable<diesel::sql_types::BigInt>, DB>,
{
  type Row = Option<i64>;
  fn build(s: Option<i64>) -> diesel::deserialize::Result<Self> {
    Ok(I64ButZeroIsNone(s.unwrap_or(0)))
  }
}

pub struct I32ButMinusOneIsNone(i32);

impl From<I32ButMinusOneIsNone> for i32 {
  fn from(x: I32ButMinusOneIsNone) -> i32 {
    x.0
  }
}

impl From<diesel::result::Error> for DbError {
  fn from(e: diesel::result::Error) -> Self {
    match e {
      diesel::result::Error::NotFound => DbError::NotFound(format!("{}", e)),
      diesel::result::Error::DatabaseError(
        diesel::result::DatabaseErrorKind::UniqueViolation,
        _,
      ) => DbError::AlreadyExists(format!("{}", e)),
      _ => DbError::Internal(format!("{:?}: {}", e, e)),
    }
  }
}

impl<DB> Queryable<diesel::sql_types::Nullable<diesel::sql_types::Integer>, DB>
  for I32ButMinusOneIsNone
where
  DB: diesel::backend::Backend,
  Option<i32>:
    diesel::deserialize::FromSql<diesel::sql_types::Nullable<diesel::sql_types::Integer>, DB>,
{
  type Row = Option<i32>;
  fn build(s: Option<i32>) -> diesel::deserialize::Result<Self> {
    Ok(I32ButMinusOneIsNone(s.unwrap_or(-1)))
  }
}

impl<DB> Queryable<diesel::sql_types::Integer, DB> for ffi::InvitationProgress
where
  DB: diesel::backend::Backend,
  i32: diesel::deserialize::FromSql<diesel::sql_types::Integer, DB>,
{
  type Row = i32;
  fn build(s: i32) -> diesel::deserialize::Result<Self> {
    match s {
      0 => Ok(ffi::InvitationProgress::OutgoingAsync),
      1 => Ok(ffi::InvitationProgress::OutgoingSync),
      2 => Ok(ffi::InvitationProgress::Complete),
      _ => Err("invalid friend request progress".into()),
    }
  }
}

use diesel::serialize::Output;
use diesel::serialize::ToSql;
impl ToSql<diesel::sql_types::Integer, diesel::sqlite::Sqlite> for ffi::InvitationProgress
where
  i32: ToSql<diesel::sql_types::Integer, diesel::sqlite::Sqlite>,
{
  fn to_sql<'b>(
    &'b self,
    out: &mut Output<'b, '_, diesel::sqlite::Sqlite>,
  ) -> diesel::serialize::Result {
    match *self {
      ffi::InvitationProgress::OutgoingAsync => out.set_value(0 as i32),
      ffi::InvitationProgress::OutgoingSync => out.set_value(1 as i32),
      ffi::InvitationProgress::Complete => out.set_value(2 as i32),
      _ => return Err("invalid friend request progress".into()),
    }
    Ok(diesel::serialize::IsNull::No)
  }
}

///
/// Source of truth is in the migrations folder. Schema.rs is generated from them.
/// This right here is just a query interface. It will not correspond exactly. It should not.
///
#[cxx::bridge(namespace = "db")]
pub mod ffi {
  //
  // WARNING: Diesel checks types. That's awesome. But IT DOES NOT CHECK THE ORDER.
  // so we need to make sure that the order here matches the order in schema.rs.
  //
  // NEVER EVER CHANGE THE ORDER OF THE FIELDS HERE WITHOUT LOOKING AT ALL QUERIES WHERE
  // THEY ARE USED. @code_review
  //
  // TODO: try to write a macro for enforcing this in code.
  //

  // THIS SHOULD BE SAME AS THE SQL VERSIONS ABOVE
  #[derive(Debug, AsExpression)]
  #[diesel(sql_type = diesel::sql_types::Integer)]
  enum InvitationProgress {
    OutgoingAsync,
    OutgoingSync,
    Complete,
  }
  #[derive(Queryable, Debug)]
  struct Friend {
    pub uid: i32,
    pub unique_name: String,
    pub display_name: String,
    #[diesel(deserialize_as = InvitationProgress)]
    pub invitation_progress: InvitationProgress,
    pub deleted: bool,
  }
  #[derive(Queryable)]
  struct CompleteFriend {
    pub uid: i32,
    pub unique_name: String,
    pub display_name: String,
    #[diesel(deserialize_as = InvitationProgress)]
    pub invitation_progress: InvitationProgress,
    pub deleted: bool,
    pub public_id: String,
    pub completed_at: i64, // unix micros
  }

  #[derive(Insertable)]
  #[diesel(table_name = crate::schema::friend)]
  struct FriendFragment {
    pub unique_name: String,
    pub display_name: String,
    pub invitation_progress: InvitationProgress,
    pub deleted: bool,
  }

  #[derive(Queryable)]
  struct Address {
    pub uid: i32,
    pub read_index: i32,
    pub read_key: Vec<u8>,
    pub write_key: Vec<u8>,
    pub ack_index: i32,
  }
  #[derive(Queryable)]
  struct OutgoingSyncInvitation {
    pub friend_uid: i32,
    pub unique_name: String,
    pub display_name: String,
    pub invitation_progress: InvitationProgress,
    pub story: String,
    pub sent_at: i64, // unix micros
  }
  #[derive(Queryable)]
  struct OutgoingAsyncInvitation {
    pub friend_uid: i32,
    pub unique_name: String,
    pub display_name: String,
    pub invitation_progress: InvitationProgress,
    pub public_id: String,
    pub invitation_public_key: Vec<u8>,
    pub kx_public_key: Vec<u8>,
    pub message: String,
    pub sent_at: i64, // unix micros
  }
  #[derive(Queryable)]
  struct JustOutgoingAsyncInvitation {
    friend_uid: i32,
    public_id: String,
    invitation_public_key: Vec<u8>,
    kx_public_key: Vec<u8>,
    message: String,
    sent_at: i64,
  }
  #[derive(Queryable)]
  struct IncomingInvitation {
    pub public_id: String,
    pub message: String,
    pub received_at: i64, // unix micros
  }

  #[derive(Queryable)]
  struct Registration {
    pub uid: i32,
    pub invitation_public_key: Vec<u8>,
    pub invitation_private_key: Vec<u8>,
    pub kx_public_key: Vec<u8>,
    pub kx_private_key: Vec<u8>,
    pub allocation: i32,
    pub pir_secret_key: Vec<u8>,
    pub pir_galois_key: Vec<u8>,
    pub authentication_token: String,
    pub public_id: String,
  }
  #[derive(Insertable)]
  #[diesel(table_name = crate::schema::registration)]
  struct RegistrationFragment {
    pub invitation_public_key: Vec<u8>,
    pub invitation_private_key: Vec<u8>,
    pub kx_public_key: Vec<u8>,
    pub kx_private_key: Vec<u8>,
    pub allocation: i32,
    pub pir_secret_key: Vec<u8>,
    pub pir_galois_key: Vec<u8>,
    pub authentication_token: String,
    pub public_id: String,
  }
  #[derive(Queryable)]
  struct SendInfo {
    pub allocation: i32,
    pub authentication_token: String,
  }
  #[derive(Queryable)]
  struct SmallRegistrationFragment {
    pub uid: i32,
    pub invitation_public_key: Vec<u8>,
    pub invitation_private_key: Vec<u8>,
    pub kx_public_key: Vec<u8>,
    pub kx_private_key: Vec<u8>,
    pub allocation: i32,
    pub authentication_token: String,
    pub public_id: String,
  }

  #[derive(Queryable, Insertable)]
  #[diesel(table_name = crate::schema::incoming_chunk)]
  struct IncomingChunk {
    pub from_friend: i32,
    pub sequence_number: i32,
    pub chunks_start_sequence_number: i32,
    pub message_uid: i32,
    pub content: String,
  }

  struct IncomingChunkFragment {
    pub from_friend: i32,
    pub sequence_number: i32,
    pub chunks_start_sequence_number: i32,
    pub content: String,
  }

  #[derive(Debug, AsExpression)]
  #[diesel(sql_type = diesel::sql_types::Integer)]
  enum SystemMessage {
    OutgoingInvitation,
  }

  #[derive(Queryable)]
  struct OutgoingChunkPlusPlus {
    pub to_friend: i32,
    pub sequence_number: i32,
    pub chunks_start_sequence_number: i32,
    #[diesel(deserialize_as = crate::db::I32ButMinusOneIsNone)]
    pub message_uid: i32, // -1 iff system message
    pub content: String,
    pub write_key: Vec<u8>,
    pub num_chunks: i32,
    pub system: bool,
    #[diesel(deserialize_as = SystemMessage)]
    pub system_message: SystemMessage,
  }

  #[derive(Queryable)]
  struct OutgoingAck {
    pub to_friend: i32,
    pub ack: i32,
    pub write_key: Vec<u8>,
    pub ack_index: i32,
  }

  enum MessageFilter {
    New,
    All,
  }
  enum DeliveryStatus {
    Delivered,
    Undelivered,
    All,
  }

  enum SortBy {
    SentAt,
    ReceivedAt,
    DeliveredAt,
    None,
  }
  struct MessageQuery {
    pub limit: i32, // use -1 to get all
    pub filter: MessageFilter,
    pub delivery_status: DeliveryStatus,
    pub sort_by: SortBy, // descending, always. newest first
    pub after: i64, // unix micros. return all messages with a sort_by strictly greater than this. use 0 to disable.
  }

  #[derive(Queryable)]
  struct ReceivedPlusPlus {
    pub uid: i32,
    pub from_unique_name: String,
    pub from_display_name: String,
    pub num_chunks: i32,
    pub received_at: i64,
    pub delivered: bool,
    #[diesel(deserialize_as = crate::db::I64ButZeroIsNone)]
    pub delivered_at: i64, // 0 iff !delivered (cxx.rs doesn't support Option)
    pub seen: bool,
    pub content: String,
  }
  #[derive(Queryable)]
  struct SentPlusPlus {
    pub uid: i32,
    pub to_unique_name: String,
    pub to_display_name: String,
    pub num_chunks: i32,
    pub sent_at: i64,
    pub delivered: bool,
    #[diesel(deserialize_as = crate::db::I64ButZeroIsNone)]
    pub delivered_at: i64, // 0 iff !delivered (cxx.rs doesn't support Option)
    pub content: String,
  }
  #[derive(Queryable)]
  struct DraftPlusPlus {
    pub uid: i32,
    pub to_unique_name: String,
    pub to_display_name: String,
    pub content: String,
  }

  #[derive(Queryable, Insertable)]
  #[diesel(table_name = crate::schema::message)]
  struct Message {
    uid: i32,
    content: String,
  }

  #[derive(Queryable, Insertable)]
  #[diesel(table_name = crate::schema::draft)]
  struct Draft {
    pub uid: i32,
    pub to_friend: i32,
  }

  enum ReceiveChunkStatus {
    NewChunk,
    NewChunkAndNewMessage,
    OldChunk,
  }

  extern "Rust" {
    type DB;
    fn init(address: &str) -> Result<Box<DB>>;

    // Debug
    unsafe fn dump(&self) -> Result<()>;

    //
    // Config
    //
    fn set_latency(&self, latency: i32) -> Result<()>;
    fn get_latency(&self) -> Result<i32>;
    fn set_server_address(&self, server_address: &str) -> Result<()>;
    fn get_server_address(&self) -> Result<String>;

    //
    // Registration
    //
    fn has_registered(&self) -> Result<bool>;
    fn get_registration(&self) -> Result<Registration>;
    fn delete_registration(&self) -> Result<()>;
    // the galois key is HUGE so we most often don't want to get it
    fn get_small_registration(&self) -> Result<SmallRegistrationFragment>;
    fn get_pir_secret_key(&self) -> Result<Vec<u8>>;
    fn get_send_info(&self) -> Result<SendInfo>;
    fn do_register(&self, reg: RegistrationFragment) -> Result<()>;

    //
    // Friends
    //
    fn get_friend(&self, unique_name: &str) -> Result<Friend>;
    // returns all friends that are complete and !deleted
    fn get_friends(&self) -> Result<Vec<CompleteFriend>>;
    fn get_friends_including_outgoing(&self) -> Result<Vec<Friend>>;
    // deletes a friend if it exists
    fn delete_friend(&self, unique_name: &str) -> Result<()>;
    // returns address iff entry in transmission table exists
    fn get_friend_address(&self, uid: i32) -> Result<Address>;
    // fails if no such friend exists
    fn get_random_enabled_friend_address_excluding(&self, uids: Vec<i32>) -> Result<Address>;

    //
    // Invitations
    //
    // currently we only support 1 async invitation at a time.
    fn has_space_for_async_invitations(&self) -> Result<bool>;
    // fails if a friend with unique_name already exists, or there are too many friends in the DB
    fn add_outgoing_sync_invitation(
      &self,
      unique_name: &str,
      display_name: &str,
      story: &str,
      kx_public_key: Vec<u8>,
      read_index: i32,
      read_key: Vec<u8>,
      write_key: Vec<u8>,
      max_friends: i32, // this is the constant defined in constants.hpp. TODO(sualeh): move it to the ffi.
    ) -> Result<Friend>;
    // fails if a friend with unique_name already exists, or there are too many friends in the DB
    // also fails if there is already an outgoing async invitation. we only support one at a time! TODO: fix this.
    fn add_outgoing_async_invitation(
      &self,
      unique_name: &str,
      display_name: &str,
      public_id: &str,
      invitation_public_key: Vec<u8>,
      kx_public_key: Vec<u8>,
      message: &str,
      read_index: i32,
      read_key: Vec<u8>,
      write_key: Vec<u8>,
      max_friends: i32, // this is the constant defined in constants.hpp. TODO(sualeh): move it to the ffi.
    ) -> Result<Friend>;
    // It is important to define the behavior of this function in the case of
    // duplicate requests. i.e. when a friend (request) with the same public key
    // is already in the database. Here's the definition for now.
    // 1. If the friend is marked as deleted, then we ignore the request.
    // 2. If the friend is marked as accepted, then we ignore the request.
    // 3. If the friend is marked as incoming, then we update the message.
    // 4. If the friend is marked as outgoing async, then we approve this request.
    // (if async outgoing, we just won't know, so we cannot take any special action)
    // immediately.
    fn add_incoming_async_invitation(&self, public_id: &str, message: &str) -> Result<()>;
    // get invitations
    fn get_outgoing_sync_invitations(&self) -> Result<Vec<OutgoingSyncInvitation>>;
    fn get_outgoing_async_invitations(&self) -> Result<Vec<OutgoingAsyncInvitation>>;
    fn get_incoming_invitations(&self) -> Result<Vec<IncomingInvitation>>;
    // accepts the incoming invitation with the given public_id
    // fails if too many friends (invitation stays in the database)
    // TODO(sualeh): move the max_friends to the ffi
    fn accept_incoming_invitation(
      &self,
      public_id: &str,
      unique_name: &str,
      display_name: &str,
      invitation_public_key: Vec<u8>,
      kx_public_key: Vec<u8>,
      read_index: i32,
      read_key: Vec<u8>,
      write_key: Vec<u8>,
      max_friends: i32,
    ) -> Result<()>;
    // simply deletes the incoming invitation
    fn deny_incoming_invitation(&self, public_id: &str) -> Result<()>;
    // receives an invitation system message, which means we should create a real friend! unless the public_id is incorrect
    fn receive_invitation_system_message(
      &self,
      from_friend: i32,
      sequence_number: i32,
      public_id: &str, // we want this public_id to correspond to the one we already have
      public_id_claimed_kx_public_key: Vec<u8>, // we want to verify that this is the same as we have on file! otherwise someone might be trying to deceive us
      public_id_claimed_invitation_public_key: Vec<u8>,
    ) -> Result<()>;

    //
    // Messages
    //
    // returns true iff the ack was novel
    fn receive_ack(&self, uid: i32, ack: i32) -> Result<bool>;
    // returns true iff a new full message was received
    fn receive_chunk(
      &self,
      chunk: IncomingChunkFragment,
      num_chunks: i32,
    ) -> Result<ReceiveChunkStatus>;
    // receive the system message telling us to add the friend

    // fails if there is no chunk to send
    // prioritizes by the given uid in order from first to last try
    // if none of the priority people have a chunk to send, pick a random chunk
    fn chunk_to_send(&self, uid_priority: Vec<i32>) -> Result<OutgoingChunkPlusPlus>;
    fn acks_to_send(&self) -> Result<Vec<OutgoingAck>>;

    // fails if the friend does not exist, or does not satisfy enabled && !deleted
    fn queue_message_to_send(
      &self,
      to_unique_name: &str,
      message: &str,
      chunks: Vec<String>,
    ) -> Result<()>;

    fn get_received_messages(&self, query: MessageQuery) -> Result<Vec<ReceivedPlusPlus>>;
    // returns 0 if no such message exists
    fn get_most_recent_received_delivered_at(&self) -> Result<i64>;
    fn get_sent_messages(&self, query: MessageQuery) -> Result<Vec<SentPlusPlus>>;
    fn get_draft_messages(&self, query: MessageQuery) -> Result<Vec<DraftPlusPlus>>;
    // no-op if already seen
    fn mark_message_as_seen(&self, uid: i32) -> Result<()>;
  }
}

pub const MIGRATIONS: diesel_migrations::EmbeddedMigrations =
  diesel_migrations::embed_migrations!();

pub fn init(address: &str) -> Result<Box<DB>, DbError> {
  let db = DB { address: address.to_string() };
  let mut conn = db.connect()?;
  use diesel_migrations::MigrationHarness;
  conn.run_pending_migrations(MIGRATIONS).map_err(|e| DbError::Unavailable(e.to_string()))?;

  db.check_rep(&mut conn);

  Ok(Box::new(db))
}

#[allow(dead_code)]
fn print_query<T: diesel::query_builder::QueryFragment<diesel::sqlite::Sqlite>>(q: &T) {
  println!("print_query: {}", diesel::debug_query::<diesel::sqlite::Sqlite, _>(&q));
}

unsafe fn errmsg_to_string(errmsg: *const std::os::raw::c_char) -> String {
  use std::ffi::CStr;
  let c_slice = CStr::from_ptr(errmsg).to_bytes();
  String::from_utf8_lossy(c_slice).into_owned()
}

impl DB {
  pub fn connect(&self) -> Result<SqliteConnection, DbError> {
    match SqliteConnection::establish(&self.address) {
      Ok(c) => Ok(c),
      Err(e) => return Err(DbError::Unknown(format!("failed to connect to database, {}", e,))),
    }
  }

  #[cfg(debug_assertions)]
  pub fn check_rep(&self, conn: &mut SqliteConnection) {
    // check_rep checks all invariants that we want to hold in the database.
    // if any of these invariants are violated, we will panic. this is okay, because check_rep is only defined in debug mode.
    // these checks may be kind of slow, in which case it might be worth exploring whether we should disable some of them in some tests
    // i definitely do not believe that they are a big bottleneck though. please profile before disabling them, because having
    // a complete check_rep that is called every single time is a HUGE bug-safety advantage.
    use crate::schema::*;
    // we unwrap everything here because we're in check_rep! so we want to fail fast.
    conn
      .transaction::<(), DbError, _>(|conn_b| {
        // invitation_progress should correspond to the correct table existing
        let friends = friend::table
          .filter(friend::deleted.eq(false))
          .select((
            friend::uid,
            friend::unique_name,
            friend::display_name,
            friend::invitation_progress,
            friend::deleted,
          ))
          .load::<ffi::Friend>(conn_b)
          .unwrap();

        for friend in friends {
          let complete_count = complete_friend::table
            .filter(complete_friend::friend_uid.eq(friend.uid))
            .count()
            .get_result::<i64>(conn_b)
            .unwrap();
          let sync_count = outgoing_sync_invitation::table
            .filter(outgoing_sync_invitation::friend_uid.eq(friend.uid))
            .count()
            .get_result::<i64>(conn_b)
            .unwrap();
          let async_count = outgoing_async_invitation::table
            .filter(outgoing_async_invitation::friend_uid.eq(friend.uid))
            .count()
            .get_result::<i64>(conn_b)
            .unwrap();
          match friend.invitation_progress {
            ffi::InvitationProgress::Complete => {
              assert!(
                complete_count == 1 && sync_count == 0 && async_count == 0,
                "In Complete invitation progress: complete_count = {}, sync_count = {}, async_count = {}",
                complete_count,
                sync_count,
                async_count
              );
            }
            ffi::InvitationProgress::OutgoingSync => {
              assert!(
                complete_count == 0 && sync_count == 1 && async_count == 0,
                "In OutgoingSync invitation progress: complete_count = {}, sync_count = {}, async_count = {}",
                complete_count,
                sync_count,
                async_count
              );
            }
            ffi::InvitationProgress::OutgoingAsync => {
              assert!(
                complete_count == 0 && sync_count == 0 && async_count == 1,
                "In OutgoingAsync invitation progress: complete_count = {}, sync_count = {}, async_count = {}",
                complete_count,
                sync_count,
                async_count
              );
            }
            _ => {
              panic!("friend with uid {} has unsupported invitation_progress", friend.uid,)
            }
          }
        }

        // message_uid is null iff system message
        let null_uid_and_not_system = outgoing_chunk::table
          .filter(outgoing_chunk::message_uid.eq::<Option<i32>>(None))
          .filter(outgoing_chunk::system.eq(false))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          null_uid_and_not_system == 0,
          "null_uid_and_not_system = {}",
          null_uid_and_not_system
        );
        let non_null_uid_and_system = outgoing_chunk::table
          .filter(outgoing_chunk::message_uid.is_not_null())
          .filter(outgoing_chunk::system.eq(true))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          non_null_uid_and_system == 0,
          "non_null_uid_and_system = {}",
          non_null_uid_and_system
        );

        // delivered_at is set iff delivered
        let delivered_at_not_null_and_delivered_false_count = sent::table
          .filter(sent::delivered_at.is_not_null())
          .filter(sent::delivered.eq(false))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          delivered_at_not_null_and_delivered_false_count == 0,
          "delivered_at_not_null_and_delivered_false_count = {}",
          delivered_at_not_null_and_delivered_false_count
        );
        let delivered_at_null_and_delivered_true_count = sent::table
          .filter(sent::delivered_at.is_null())
          .filter(sent::delivered.eq(true))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          delivered_at_null_and_delivered_true_count == 0,
          "delivered_at_null_and_delivered_true_count = {}",
          delivered_at_null_and_delivered_true_count
        );
        // same for the received table
        let delivered_at_not_null_and_delivered_false_count = received::table
          .filter(received::delivered_at.is_not_null())
          .filter(received::delivered.eq(false))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          delivered_at_not_null_and_delivered_false_count == 0,
          "delivered_at_not_null_and_delivered_false_count = {}",
          delivered_at_not_null_and_delivered_false_count
        );
        let delivered_at_null_and_delivered_true_count = received::table
          .filter(received::delivered_at.is_null())
          .filter(received::delivered.eq(true))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          delivered_at_null_and_delivered_true_count == 0,
          "delivered_at_null_and_delivered_true_count = {}",
          delivered_at_null_and_delivered_true_count
        );

        // draft, sent, received are mutually disjoint
        let draft_and_sent_count = draft::table
          .inner_join(sent::table.on(draft::uid.eq(sent::uid)))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          draft_and_sent_count == 0,
          "draft_and_sent_count = {}",
          draft_and_sent_count
        );
        let draft_and_received_count = draft::table
          .inner_join(received::table.on(draft::uid.eq(received::uid)))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          draft_and_received_count == 0,
          "draft_and_received_count = {}",
          draft_and_received_count
        );
        let sent_and_received_count = sent::table
          .inner_join(received::table.on(sent::uid.eq(received::uid)))
          .count()
          .get_result::<i64>(conn_b)
          .unwrap();
        assert!(
          sent_and_received_count == 0,
          "sent_and_received_count = {}",
          sent_and_received_count
        );

        // exactly 1 config always
        let config_count = config::table.count().get_result::<i64>(conn_b).unwrap();
        assert!(config_count == 1, "config_count = {}", config_count);

        // if has_registered is true, then should have exactly 1 registration
        let has_registered = config::table.select(config::has_registered).first::<bool>(conn_b).unwrap();
        let registration_count = registration::table.count().get_result::<i64>(conn_b).unwrap();
        if has_registered {
          assert!(registration_count == 1, "registration_count = {}, has_registered = {}", registration_count, has_registered);
        } else {
          assert!(registration_count == 0, "registration_count = {}, has_registered = {}", registration_count, has_registered);
        }

        // transmission table iff !deleted
        let transmission_non_deleted_count = friend::table.filter(friend::deleted.eq(false)).inner_join(transmission::table).count().get_result::<i64>(conn_b).unwrap();
        let non_deleted_count = friend::table.filter(friend::deleted.eq(false)).count().get_result::<i64>(conn_b).unwrap();
        let transmission_count = transmission::table.count().get_result::<i64>(conn_b).unwrap();
        assert!(
          transmission_non_deleted_count == non_deleted_count,
          "transmission_non_deleted_count = {}, non_deleted_count = {}",
          transmission_non_deleted_count,
          non_deleted_count
        );
        assert!(
          transmission_count == transmission_non_deleted_count,
          "transmission_count = {}, transmission_non_deleted_count = {}",
          transmission_count,
          transmission_non_deleted_count
        );

        // outgoing chunk (to_friend, chunks_start_sequence_number) is in a 1-to-1 relationship with (to_friend, message_uid) (except for nulls in message_uid)
        // diesel doesn't support aliases, so we have to do this manually by first getting all chunks and then processing them
        let chunks = outgoing_chunk::table.select((
          outgoing_chunk::to_friend,
          outgoing_chunk::chunks_start_sequence_number,
          outgoing_chunk::message_uid,
        )).get_results::<ChunkInterestingNumbers>(conn_b).unwrap();
        let mut to_friend_seqnum_map_to_message_uid = HashMap::new();
        for chunk in &chunks {
          let to_friend = chunk.to_friend;
          let seqnum = chunk.chunks_start_sequence_number;
          let message_uid = chunk.message_uid;
          if !to_friend_seqnum_map_to_message_uid.contains_key(&(to_friend, seqnum)) {
            to_friend_seqnum_map_to_message_uid.insert((to_friend, seqnum), message_uid);
          } else {
            assert!(
              to_friend_seqnum_map_to_message_uid.get(&(to_friend, seqnum)).unwrap() == &message_uid,
              "chunk = {:?}",
              chunk
            );
          }
        }
        let mut to_friend_message_uid_map_to_seqnum = HashMap::new();
        for chunk in &chunks {
          let to_friend = chunk.to_friend;
          let message_uid = chunk.message_uid;
          let seqnum = chunk.chunks_start_sequence_number;
          if message_uid.is_none() {
            // message_uid is none, so we don't care about seqnum here
            continue;
          }
          if !to_friend_message_uid_map_to_seqnum.contains_key(&(to_friend, message_uid)) {
            to_friend_message_uid_map_to_seqnum.insert((to_friend, message_uid), seqnum);
          } else {
            assert!(
              to_friend_message_uid_map_to_seqnum.get(&(to_friend, message_uid)).unwrap() == &seqnum,
              "chunk = {:?}",
              chunk
            );
          }
        }
        // the same thing should be true for the incoming chunks
        let chunks = incoming_chunk::table.select((
          incoming_chunk::from_friend,
          incoming_chunk::chunks_start_sequence_number,
          incoming_chunk::message_uid,
        )).get_results::<IncomingChunkInterestingNumbers>(conn_b).unwrap();
        let mut from_friend_seqnum_map_to_message_uid = HashMap::new();
        for chunk in &chunks {
          let from_friend = chunk.from_friend;
          let seqnum = chunk.chunks_start_sequence_number;
          let message_uid = chunk.message_uid;
          if !from_friend_seqnum_map_to_message_uid.contains_key(&(from_friend, seqnum)) {
            from_friend_seqnum_map_to_message_uid.insert((from_friend, seqnum), message_uid);
          } else {
            assert!(
              from_friend_seqnum_map_to_message_uid.get(&(from_friend, seqnum)).unwrap() == &message_uid,
              "chunk = {:?}",
              chunk
            );
          }
        }
        let mut from_friend_message_uid_map_to_seqnum = HashMap::new();
        for chunk in &chunks {
          let from_friend = chunk.from_friend;
          let message_uid = chunk.message_uid;
          let seqnum = chunk.chunks_start_sequence_number;
          if !from_friend_message_uid_map_to_seqnum.contains_key(&(from_friend, message_uid)) {
            from_friend_message_uid_map_to_seqnum.insert((from_friend, message_uid), seqnum);
          } else {
            assert!(
              from_friend_message_uid_map_to_seqnum.get(&(from_friend, message_uid)).unwrap() == &seqnum,
              "chunk = {:?}",
              chunk
            );
          }
        }

        // ack_index must always be >= 0
        let ack_index_count = transmission::table.filter(transmission::ack_index.lt(0)).count().get_result::<i64>(conn_b).unwrap();
        assert!(ack_index_count == 0, "ack_index_count = {}", ack_index_count);


        Ok(())
      })
      .unwrap();
  }

  #[cfg(not(debug_assertions))]
  pub fn check_rep(&self, conn: &mut SqliteConnection) {
    ()
  }

  /// # Safety
  ///
  /// This dumps the database using pure sql which uses pointers written by hand. Possibly memory leaks.
  /// Do not call this function inside of a production environment.
  #[allow(dead_code)]
  pub unsafe fn dump(&self) -> Result<(), DbError> {
    use libsqlite3_sys::*;
    use std::ffi::CString;
    use std::fmt::Write;

    println!("------ Dumping DB ------");

    let mut db: *mut sqlite3 = std::ptr::null_mut();
    let c_address = CString::new(self.address.clone()).unwrap();
    let success =
      sqlite3_open_v2(c_address.as_ptr(), &mut db, SQLITE_OPEN_READONLY, std::ptr::null());

    if success != SQLITE_OK {
      return Err(DbError::Unknown(errmsg_to_string(sqlite3_errmsg(db))));
    }

    let mut raw_smt: *mut sqlite3_stmt = std::ptr::null_mut();
    let c_query = CString::new(
      "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'",
    )
    .unwrap();
    let prepare_result =
      sqlite3_prepare_v2(db, c_query.as_ptr(), 500, &mut raw_smt, std::ptr::null_mut());
    if prepare_result != SQLITE_OK {
      return Err(DbError::Unknown(errmsg_to_string(sqlite3_errmsg(db))));
    }

    let mut table_names = Vec::new();
    while sqlite3_step(raw_smt) == SQLITE_ROW {
      let table_name = std::ffi::CStr::from_ptr(sqlite3_column_text(raw_smt, 0) as *const i8)
        .to_string_lossy()
        .into_owned();
      table_names.push(table_name);
    }

    sqlite3_finalize(raw_smt);

    for table_name in table_names {
      println!("{}", table_name);

      let query: String = format!("SELECT * FROM {}", table_name);
      let c_query = CString::new(query).unwrap();

      let mut raw_stmt: *mut sqlite3_stmt = std::ptr::null_mut();
      let prepare_result =
        sqlite3_prepare_v2(db, c_query.as_ptr(), 500, &mut raw_stmt, std::ptr::null_mut());
      if prepare_result != SQLITE_OK {
        return Err(DbError::Unknown(errmsg_to_string(sqlite3_errmsg(db))));
      }

      while sqlite3_step(raw_stmt) == SQLITE_ROW {
        let mut s = String::new();
        // print the row, and handle null values
        for i in 0..sqlite3_column_count(raw_stmt) {
          if sqlite3_column_type(raw_stmt, i) == SQLITE_NULL {
            write!(s, "NULL").unwrap();
          } else {
            let c_str = std::ffi::CStr::from_ptr(sqlite3_column_text(raw_stmt, i) as *const i8);
            write!(s, "{}", c_str.to_string_lossy()).unwrap();
          }
          write!(s, " ").unwrap();
        }
        println!(">> {}", s);
        println!();
      }

      sqlite3_finalize(raw_stmt);
    }

    println!("------ Dump complete ------");

    Ok(())
  }

  pub fn has_registered(&self) -> Result<bool, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::config;

    self.check_rep(&mut conn);

    let q = config::table.select(config::has_registered);
    let has_registered = q
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query has_registered: {}", e)))?;
    Ok(has_registered)
  }

  pub fn do_register(&self, reg: ffi::RegistrationFragment) -> Result<(), DbError> {
    let mut conn = self.connect()?;

    self.check_rep(&mut conn);

    use crate::schema::config;
    use crate::schema::registration;

    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let count = registration::table.count().get_result::<i64>(conn_b)?;
      if count > 0 {
        return Err(diesel::result::Error::RollbackTransaction);
      }

      let reg_uid = diesel::insert_into(registration::table)
        .values(&reg)
        .returning(registration::uid)
        .get_result::<i32>(conn_b)?;
      // update the config table
      diesel::update(config::table)
        .set((config::has_registered.eq(true), config::registration_uid.eq(reg_uid)))
        .execute(conn_b)?;

      Ok(())
    });

    match r {
      Ok(_) => Ok(()),
      Err(diesel::result::Error::RollbackTransaction) => {
        Err(DbError::AlreadyExists("registration already exists".to_string()))
      }
      Err(e) => Err(DbError::Unknown(format!("failed to insert registration: {}", e))),
    }
  }

  pub fn get_registration(&self) -> Result<ffi::Registration, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::registration;

    self.check_rep(&mut conn);

    let registration = registration::table
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query registration: {}", e)))?;
    Ok(registration)
  }

  pub fn delete_registration(&self) -> Result<(), DbError> {
    let mut conn = self.connect()?;

    self.check_rep(&mut conn);

    use crate::schema::config;
    use crate::schema::registration;

    conn
      .transaction::<_, diesel::result::Error, _>(|conn_b| {
        diesel::update(config::table)
          .set((config::has_registered.eq(false), config::registration_uid.eq::<Option<i32>>(None)))
          .execute(conn_b)?;

        diesel::delete(registration::table).execute(conn_b)?;
        Ok(())
      })
      .map_err(|e| DbError::Unknown(format!("failed to delete registration: {}", e)))?;

    self.check_rep(&mut conn);

    Ok(())
  }

  pub fn get_small_registration(&self) -> Result<ffi::SmallRegistrationFragment, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::registration;

    self.check_rep(&mut conn);

    let q = registration::table.select((
      registration::uid,
      registration::invitation_public_key,
      registration::invitation_private_key,
      registration::kx_public_key,
      registration::kx_private_key,
      registration::allocation,
      registration::authentication_token,
      registration::public_id,
    ));
    let registration = q
      .first::<ffi::SmallRegistrationFragment>(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query registration: {}", e)))?;
    Ok(registration)
  }

  pub fn get_pir_secret_key(&self) -> Result<Vec<u8>, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::registration;

    self.check_rep(&mut conn);

    let q = registration::table.select(registration::pir_secret_key);
    let pir_secret_key = q
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query pir_secret_key: {}", e)))?;
    Ok(pir_secret_key)
  }

  pub fn get_send_info(&self) -> Result<ffi::SendInfo, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::registration;

    self.check_rep(&mut conn);

    let q =
      registration::table.select((registration::allocation, registration::authentication_token));
    let send_info = q
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query send_info: {}", e)))?;
    Ok(send_info)
  }

  pub fn get_friend(&self, unique_name: &str) -> Result<ffi::Friend, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::friend;

    self.check_rep(&mut conn);

    if let Ok(f) =
      friend::table.filter(friend::unique_name.eq(unique_name)).first::<ffi::Friend>(&mut conn)
    {
      Ok(f)
    } else {
      Err(DbError::NotFound("failed to get friend".to_string()))
    }
  }

  pub fn get_friends(&self) -> Result<Vec<ffi::CompleteFriend>, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::complete_friend;
    use crate::schema::friend;

    self.check_rep(&mut conn);

    // only get complete friends
    if let Ok(v) = friend::table
      .filter(friend::invitation_progress.eq(ffi::InvitationProgress::Complete))
      .filter(friend::deleted.eq(false))
      .inner_join(complete_friend::table)
      .select((
        friend::uid,
        friend::unique_name,
        friend::display_name,
        friend::invitation_progress,
        friend::deleted,
        complete_friend::public_id,
        complete_friend::completed_at,
      ))
      .load::<ffi::CompleteFriend>(&mut conn)
    {
      Ok(v)
    } else {
      Err(DbError::Unknown("failed to get friends".to_string()))
    }
  }

  pub fn get_friends_including_outgoing(&self) -> Result<Vec<ffi::Friend>, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::friend;

    self.check_rep(&mut conn);

    if let Ok(v) = friend::table
      .filter(friend::deleted.eq(false))
      .select((
        friend::uid,
        friend::unique_name,
        friend::display_name,
        friend::invitation_progress,
        friend::deleted,
      ))
      .load::<ffi::Friend>(&mut conn)
    {
      Ok(v)
    } else {
      Err(DbError::Unknown("failed to get friends".to_string()))
    }
  }

  pub fn delete_friend(&self, unique_name: &str) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    use crate::schema::friend;

    self.check_rep(&mut conn);

    diesel::update(friend::table.filter(friend::unique_name.eq(unique_name)))
      .set(friend::deleted.eq(false))
      .returning(friend::uid)
      .get_result::<i32>(&mut conn)
      .map_err(|e| match e {
        diesel::result::Error::NotFound => DbError::NotFound("friend not found".to_string()),
        _ => DbError::Unknown(format!("failed to delete friend: {}", e)),
      })?;

    self.check_rep(&mut conn);

    Ok(())
  }

  pub fn set_latency(&self, latency: i32) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    use crate::schema::config;

    self.check_rep(&mut conn);

    let r = diesel::update(config::table).set(config::latency.eq(latency)).execute(&mut conn);

    self.check_rep(&mut conn);

    match r {
      Ok(_) => Ok(()),
      Err(e) => Err(DbError::Unknown(format!("set_latency: {}", e))),
    }
  }

  pub fn get_latency(&self) -> Result<i32, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::config;

    self.check_rep(&mut conn);

    let q = config::table.select(config::latency);
    let latency = q
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query latency: {}", e)))?;
    Ok(latency)
  }

  pub fn set_server_address(&self, server_address: &str) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    use crate::schema::config;

    self.check_rep(&mut conn);

    let r = diesel::update(config::table)
      .set(config::server_address.eq(server_address))
      .execute(&mut conn);

    match r {
      Ok(_) => Ok(()),
      Err(e) => Err(DbError::Unknown(format!("set_server_address: {}", e))),
    }
  }

  pub fn get_server_address(&self) -> Result<String, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::config;

    self.check_rep(&mut conn);

    match config::table.select(config::server_address).first(&mut conn) {
      Ok(server_address) => Ok(server_address),
      Err(e) => Err(DbError::Unknown(format!("get_server_address: {}", e))),
    }
  }

  pub fn receive_ack(&self, uid: i32, ack: i32) -> Result<bool, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::outgoing_chunk;
    use crate::schema::sent;
    use crate::schema::transmission;
    use crate::schema::friend;
    use crate::schema::complete_friend;
    use crate::schema::outgoing_async_invitation;

    self.check_rep(&mut conn);

    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let old_acked_seqnum =
        transmission::table.find(uid).select(transmission::sent_acked_seqnum).first(conn_b)?;
      if ack > old_acked_seqnum {
        diesel::update(transmission::table.find(uid))
          .set(transmission::sent_acked_seqnum.eq(ack))
          .execute(conn_b)?;
        // Special case: this is an ACK to an outgoing request system message.
        let chunk_acked : (bool, ffi::SystemMessage) = outgoing_chunk::table
          .filter(outgoing_chunk::to_friend.eq(uid))
          .filter(outgoing_chunk::sequence_number.eq(ack))
          .select((outgoing_chunk::system,
                  outgoing_chunk::system_message))
          .first::<(bool, ffi::SystemMessage)>(conn_b)?;
        if (chunk_acked.0, chunk_acked.1) == (true, ffi::SystemMessage::OutgoingInvitation) {
          // This is a system control message for an outgoing invitation.
          // In this case, we become complete friends with the other party.
          // ======================
          // To deduplicate, we first check that the uid is in the outgoing async table
          let invite = outgoing_async_invitation::table
          .filter(outgoing_async_invitation::friend_uid.eq(uid))
          .load::<ffi::JustOutgoingAsyncInvitation>(conn_b)?;
          // if invite.len() == 0, then we need to do nothing
          // since the invite has already been approved before
          if invite.len() == 1 {
            let invite = invite.first().unwrap();
            // we update the friend progress to be complete
            diesel::update(friend::table.filter(friend::uid.eq(uid)))
              .set(friend::invitation_progress.eq(ffi::InvitationProgress::Complete))
              .execute(conn_b)?;
            // we push the friend into complete_friend table
            // to achieve that, we need to obtain several keys.
            // fortunately, these keys are already in the invitation table.

            diesel::insert_into(complete_friend::table)
              .values((
                complete_friend::friend_uid.eq(uid),
                complete_friend::public_id.eq(invite.public_id.clone()),
                complete_friend::invitation_public_key.eq(invite.invitation_public_key.clone()),
                complete_friend::kx_public_key.eq(invite.kx_public_key.clone()),
                complete_friend::completed_at.eq(util::unix_micros_now()),
              ))
              .execute(conn_b)?;
            // remove the invite from the table
            diesel::delete(
              outgoing_async_invitation::table.filter(
              outgoing_async_invitation::friend_uid.eq(uid))
            ).execute(conn_b)?;
          }
          // ======================
        }

        // delete all outgoing chunks with seqnum <= ack
        diesel::delete(
          outgoing_chunk::table
            .filter(outgoing_chunk::to_friend.eq(uid))
            .filter(outgoing_chunk::sequence_number.le(ack)),
        )
        .execute(conn_b)?;
        // potentially transition messages to delivered status!
        // when? when there are messages in received that aren't
        // delivered but also do not have incoming chunks
        let newly_delivered = sent::table
          .filter(sent::to_friend.eq(uid))
          .filter(sent::delivered.eq(false))
          .left_outer_join(outgoing_chunk::table)
          .filter(outgoing_chunk::sequence_number.nullable().is_null())
          .select(sent::uid)
          .load::<i32>(conn_b)?;


        // we use ii to make sure that times are guaranteed to be unique
        for (ii, uid) in (0_i64..).zip(newly_delivered.into_iter()) {
          diesel::update(sent::table.find(uid))
            .set((sent::delivered.eq(true), sent::delivered_at.eq(util::unix_micros_now() + ii)))
            .execute(conn_b)?;
        }
      }
      Ok(old_acked_seqnum)
    });

    self.check_rep(&mut conn);

    match r {
      Ok(old_ack) => match ack {
        ack if ack > old_ack => Ok(true),
        ack if ack == old_ack => Ok(false),
        _ => {
          println!("Ack is older than old ack. This is weird, and probably indicates that the person you're talking to has done something wrong.");
          Ok(false)
        }
      },
      // match old_ack
      Err(e) => Err(DbError::Unknown(format!("receive_ack: {}", e))),
    }
  }

  pub fn update_sequence_number(
    &self,
    conn: &mut SqliteConnection,
    from_friend: i32,
    sequence_number: i32,
  ) -> Result<ffi::ReceiveChunkStatus, diesel::result::Error> {
    use crate::schema::transmission;

    conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let old_seqnum = transmission::table
        .find(from_friend)
        .select(transmission::received_seqnum)
        .first::<i32>(conn_b)?;
      // if chunk is before old_seqnum, we just ignore it!!
      // in other words, once a chunk has been received, it can never be patched.
      // this is good. if something bad happens, you can always retransmit in a new
      // message.
      if sequence_number <= old_seqnum {
        return Ok(ffi::ReceiveChunkStatus::OldChunk);
      }
      // we want to update received_seqnum in status!

      // so we only update the seqnum if we increase exactly by one (otherwise we might miss messages!)
      if sequence_number == old_seqnum + 1 {
        diesel::update(transmission::table.find(from_friend))
          .set(transmission::received_seqnum.eq(sequence_number))
          .execute(conn_b)?;
      }
      Ok(ffi::ReceiveChunkStatus::NewChunk)
    })
  }

  pub fn receive_chunk(
    &self,
    chunk: ffi::IncomingChunkFragment,
    num_chunks: i32,
  ) -> Result<ffi::ReceiveChunkStatus, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::incoming_chunk;
    use crate::schema::message;
    use crate::schema::received;

    self.check_rep(&mut conn);

    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let chunk_status =
        self.update_sequence_number(conn_b, chunk.from_friend, chunk.sequence_number)?;
      if chunk_status == ffi::ReceiveChunkStatus::OldChunk {
        return Ok(chunk_status);
      }

      // check if there is already a message uid associated with this chunk sequence
      let q =
        incoming_chunk::table.filter(incoming_chunk::from_friend.eq(chunk.from_friend).and(
          incoming_chunk::chunks_start_sequence_number.eq(chunk.chunks_start_sequence_number),
        ));
      let message_uid;
      match q.first::<ffi::IncomingChunk>(conn_b) {
        Ok(ref_chunk) => {
          message_uid = ref_chunk.message_uid;
          // ok now we want to simply insert this chunk!
          let insertable_chunk = ffi::IncomingChunk {
            from_friend: chunk.from_friend,
            sequence_number: chunk.sequence_number,
            chunks_start_sequence_number: chunk.chunks_start_sequence_number,
            message_uid,
            content: chunk.content,
          };
          diesel::insert_into(incoming_chunk::table).values(&insertable_chunk).execute(conn_b)?;
        }
        Err(_) => {
          // if there is no message uid associated with this chunk sequence, we need to create a new message
          let new_msg = diesel::insert_into(message::table)
            .values(message::content.eq(""))
            .get_result::<ffi::Message>(conn_b)?;

          message_uid = new_msg.uid;
          let new_received = Received {
            uid: message_uid,
            from_friend: chunk.from_friend,
            num_chunks,
            received_at: util::unix_micros_now(),
            delivered: false,
            delivered_at: None,
            seen: false,
          };
          diesel::insert_into(received::table).values(&new_received).execute(conn_b)?;

          let insertable_chunk = ffi::IncomingChunk {
            from_friend: chunk.from_friend,
            sequence_number: chunk.sequence_number,
            chunks_start_sequence_number: chunk.chunks_start_sequence_number,
            message_uid,
            content: chunk.content,
          };
          diesel::insert_into(incoming_chunk::table).values(&insertable_chunk).execute(conn_b)?;
        }
      };

      // check if we have received all chunks!
      let q =
        incoming_chunk::table.filter(incoming_chunk::from_friend.eq(chunk.from_friend).and(
          incoming_chunk::chunks_start_sequence_number.eq(chunk.chunks_start_sequence_number),
        ));

      let count: i64 = q.count().get_result(conn_b)?;
      if count == num_chunks as i64 {
        // we have received all chunks!
        // now assemble the message, write it, and be happy!
        let all_chunks =
          q.order_by(incoming_chunk::sequence_number).load::<ffi::IncomingChunk>(conn_b)?;
        let msg = all_chunks.iter().fold(String::new(), |mut acc, chunk| {
          acc.push_str(&chunk.content);
          acc
        });
        diesel::update(message::table.find(message_uid))
          .set((message::content.eq(msg),))
          .execute(conn_b)?;
        // update the receive table
        diesel::update(received::table.find(message_uid))
          .set((received::delivered.eq(true), received::delivered_at.eq(util::unix_micros_now())))
          .execute(conn_b)?;
        // finally, delete the chunks
        diesel::delete(incoming_chunk::table.filter(
          incoming_chunk::from_friend.eq(chunk.from_friend).and(
            incoming_chunk::chunks_start_sequence_number.eq(chunk.chunks_start_sequence_number),
          ),
        ))
        .execute(conn_b)?;
        return Ok(ffi::ReceiveChunkStatus::NewChunkAndNewMessage);
      }

      Ok(ffi::ReceiveChunkStatus::NewChunk)
    });

    self.check_rep(&mut conn);

    match r {
      Ok(b) => Ok(b),
      Err(e) => Err(DbError::Unknown(format!("receive_chunk: {}", e))),
    }
  }

  pub fn chunk_to_send(
    &self,
    uid_priority: Vec<i32>,
  ) -> Result<ffi::OutgoingChunkPlusPlus, anyhow::Error> {
    let mut conn = self.connect()?;

    self.check_rep(&mut conn);

    use crate::schema::friend;
    use crate::schema::outgoing_chunk;
    use crate::schema::sent;
    use crate::schema::transmission;

    // We could do probably this in one query, by joining on the select statement
    // and then joining. Diesel doesn't typecheck this, and maybe it is unsafe, so
    // let's just do a transaction.
    let r = conn.transaction::<_, anyhow::Error, _>(|conn_b| {
      let q = outgoing_chunk::table
        .group_by(outgoing_chunk::to_friend)
        .select((outgoing_chunk::to_friend, diesel::dsl::min(outgoing_chunk::sequence_number)));
      let first_chunk_per_friend: Vec<(i32, Option<i32>)> = q.load::<(i32, Option<i32>)>(conn_b)?;
      let mut first_chunk_per_friend: Vec<(i32, i32)> =
        first_chunk_per_friend.iter().fold(vec![], |mut acc, &x| {
          match x.1 {
            Some(seq) => acc.push((x.0, seq)),
            None => (),
          };
          acc
        });
      if first_chunk_per_friend.is_empty() {
        return Err(diesel::result::Error::NotFound).map_err(|e| e.into());
      }
      let chosen_chunk: (i32, i32) = (|| {
        for uid in uid_priority {
          if let Some(index) = first_chunk_per_friend.iter().position(|c| c.0 == uid) {
            return first_chunk_per_friend.remove(index);
          }
        }
        let rng: usize = rand::random();
        let index = rng % first_chunk_per_friend.len();
        first_chunk_per_friend.remove(index)
      })();
      // special case: control message
      let is_system_message = outgoing_chunk::table
        .find(chosen_chunk)
        .select(outgoing_chunk::system)
        .first::<bool>(conn_b)?;
      if is_system_message {
        // the number of chunks for system messages is always 1
        // unfortunately, system messages do not have an entry in the sent table
        // so the code for non-system messages do not work.
        let chunk_plusplus_minusnumchunks = outgoing_chunk::table
          .find(chosen_chunk)
          .inner_join(friend::table.inner_join(transmission::table))
          .select((
            outgoing_chunk::to_friend,
            outgoing_chunk::sequence_number,
            outgoing_chunk::chunks_start_sequence_number,
            outgoing_chunk::message_uid,
            outgoing_chunk::content,
            transmission::write_key,
            outgoing_chunk::system,
            outgoing_chunk::system_message,
          ))
          .first::<OutgoingChunkPlusPlusMinusNumChunks>(conn_b)
          .context("chunk_to_send, cannot find chosen chunk in the outgoing_chunk table")?;
        let chunk_plusplus = ffi::OutgoingChunkPlusPlus {
          to_friend: chunk_plusplus_minusnumchunks.to_friend,
          sequence_number: chunk_plusplus_minusnumchunks.sequence_number,
          chunks_start_sequence_number: chunk_plusplus_minusnumchunks.chunks_start_sequence_number,
          message_uid: chunk_plusplus_minusnumchunks.message_uid,
          content: chunk_plusplus_minusnumchunks.content,
          write_key: chunk_plusplus_minusnumchunks.write_key,
          num_chunks: 1,
          system: chunk_plusplus_minusnumchunks.system,
          system_message: chunk_plusplus_minusnumchunks.system_message,
        };
        Ok(chunk_plusplus)
      } else {
        let chunk_plusplus = outgoing_chunk::table
          .find(chosen_chunk)
          .inner_join(friend::table.inner_join(transmission::table))
          .inner_join(sent::table)
          .select((
            outgoing_chunk::to_friend,
            outgoing_chunk::sequence_number,
            outgoing_chunk::chunks_start_sequence_number,
            outgoing_chunk::message_uid,
            outgoing_chunk::content,
            transmission::write_key,
            sent::num_chunks,
            outgoing_chunk::system,
            outgoing_chunk::system_message,
          ))
          .first::<ffi::OutgoingChunkPlusPlus>(conn_b)
          .context("chunk_to_send, cannot find chosen chunk in the outgoing_chunk table")?;
        Ok(chunk_plusplus)
      }
    });

    self.check_rep(&mut conn);

    r
  }

  pub fn acks_to_send(&self) -> Result<Vec<ffi::OutgoingAck>, DbError> {
    let mut conn = self.connect()?;

    self.check_rep(&mut conn);

    use crate::schema::friend;
    use crate::schema::transmission;
    let wide_friends =
      friend::table.filter(friend::deleted.eq(false)).inner_join(transmission::table);
    let q = wide_friends.select((
      friend::uid,
      transmission::received_seqnum,
      transmission::write_key,
      transmission::ack_index,
    ));
    let r = q.load::<ffi::OutgoingAck>(&mut conn);
    match r {
      Ok(acks) => Ok(acks),
      Err(e) => Err(DbError::Unknown(format!("acks_to_send: {}", e))),
    }
  }

  pub fn get_friend_address(&self, uid: i32) -> Result<ffi::Address, DbError> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);

    use crate::schema::transmission;

    match transmission::table
      .find(uid)
      .select((
        transmission::friend_uid,
        transmission::read_index,
        transmission::read_key,
        transmission::write_key,
        transmission::ack_index,
      ))
      .first(&mut conn)
    {
      Ok(address) => Ok(address),
      Err(e) => Err(DbError::Unknown(format!("get_friend_address: {}", e))),
    }
  }

  pub fn get_random_enabled_friend_address_excluding(
    &self,
    uids: Vec<i32>,
  ) -> Result<ffi::Address, DbError> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);
    use crate::schema::friend;
    use crate::schema::transmission;

    // get a random friend that is not deleted excluding the ones in the uids list
    // we alow getting any outgoing friend too. they should be treated normally
    let q = friend::table.filter(friend::deleted.eq(false));

    // Inner join to get the (friend, address) pairs and then select the address.
    let q = q.inner_join(transmission::table).select((
      transmission::friend_uid,
      transmission::read_index,
      transmission::read_key,
      transmission::write_key,
      transmission::ack_index,
    ));
    let addresses = q.load::<ffi::Address>(&mut conn);

    match addresses {
      Ok(addresses) => {
        let rng: usize = rand::random();
        let mut filtered_addresses =
          addresses.into_iter().filter(|address| !uids.contains(&address.uid)).collect::<Vec<_>>();
        if filtered_addresses.is_empty() {
          return Err(DbError::NotFound(
            "No enabled friends not in the excluded list found.".to_string(),
          ));
        }
        let index = rng % filtered_addresses.len();
        Ok(filtered_addresses.remove(index))
      }
      Err(e) => {
        Err(DbError::Unknown(format!("get_random_enabled_friend_address_excluding: {}", e)))
      }
    }
  }

  fn get_friend_uid_by_unique_name(
    &self,
    conn: &mut SqliteConnection,
    unique_name: &str,
  ) -> Result<i32, diesel::result::Error> {
    use crate::schema::friend;

    let q = friend::table.filter(friend::unique_name.eq(unique_name)).select(friend::uid);
    q.first(conn)
  }

  // input: the uid of a friend.
  // output: the next sequence number to use for sending a message to that friend.
  //         this is equal to the previous seqnum + 1,
  //         or 1 if no previous message exist
  fn get_seqnum_for_new_chunk(
    &self,
    conn: &mut SqliteConnection,
    friend_uid: i32,
  ) -> Result<i32, anyhow::Error> {
    use crate::schema::outgoing_chunk;
    use crate::schema::transmission;
    conn.transaction::<_, anyhow::Error, _>(|conn_b| {
      // get the correct sequence number. This is the last sequence number + 1.
      // this is either the maximum sequence number in the table + 1,
      // or the sent_acked_seqnum + 1 if there are no outgoing chunks.
      let maybe_old_seqnum = outgoing_chunk::table
        .filter(outgoing_chunk::to_friend.eq(friend_uid))
        .select(outgoing_chunk::sequence_number)
        .order_by(outgoing_chunk::sequence_number.desc())
        .limit(1)
        .load::<i32>(conn_b)
        .context("get_seqnum_for_new_chunk, failed to find old sequence number from the outgoing chunk table")?;
      let old_seqnum = match maybe_old_seqnum.len() {
        0 => transmission::table
          .find(friend_uid)
          .select(transmission::sent_acked_seqnum)
          .first::<i32>(conn_b).context("get_seqnum_for_new_chunk, failed to find seqnum from the transmission table")?,
        _ => maybe_old_seqnum[0],
      };
      let new_seqnum = old_seqnum + 1;
      Ok(new_seqnum)
    })
  }

  pub fn queue_message_to_send(
    &self,
    to_unique_name: &str,
    message: &str,
    chunks: Vec<String>,
  ) -> Result<(), DbError> {
    // We chunk in C++ because we potentially need to do things with protobuf
    // What do we do here?
    // 1. Create a new message.
    // 2. Create a new sent message.
    // 3. Take in all chunks, create outgoing_chunks.
    let mut conn = self.connect()?;
    use crate::schema::message;
    use crate::schema::outgoing_chunk;
    use crate::schema::sent;

    self.check_rep(&mut conn);

    conn
      .transaction::<_, anyhow::Error, _>(|conn_b| {
        let friend_uid = self.get_friend_uid_by_unique_name(conn_b, to_unique_name)?;

        let message_uid = diesel::insert_into(message::table)
          .values((message::content.eq(message),))
          .returning(message::uid)
          .get_result::<i32>(conn_b)?;

        diesel::insert_into(sent::table)
          .values((
            sent::uid.eq(message_uid),
            sent::to_friend.eq(friend_uid),
            sent::num_chunks.eq(chunks.len() as i32),
            sent::sent_at.eq(util::unix_micros_now()),
            sent::delivered.eq(false),
          ))
          .execute(conn_b)?;

        let new_seqnum = self.get_seqnum_for_new_chunk(conn_b, friend_uid)?;

        for (i, chunk) in chunks.iter().enumerate() {
          diesel::insert_into(outgoing_chunk::table)
            .values((
              outgoing_chunk::to_friend.eq(friend_uid),
              outgoing_chunk::sequence_number.eq(new_seqnum + i as i32),
              outgoing_chunk::chunks_start_sequence_number.eq(new_seqnum),
              outgoing_chunk::message_uid.eq(message_uid),
              outgoing_chunk::content.eq(chunk),
              outgoing_chunk::system.eq(false),
              outgoing_chunk::system_message.eq(ffi::SystemMessage::OutgoingInvitation), // doesn't matter, we don't use this if control is false anyway
            ))
            .execute(conn_b)?;
        }

        Ok(())
      })
      .map_err(|e| {
        DbError::Unknown(format!("queue_message_to_send: {} (maybe friend doesn't exist?)", e))
      })?;

    self.check_rep(&mut conn);

    Ok(())
  }

  pub fn get_received_messages(
    &self,
    query: ffi::MessageQuery,
  ) -> Result<Vec<ffi::ReceivedPlusPlus>, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::friend;
    use crate::schema::message;
    use crate::schema::received;
    self.check_rep(&mut conn);

    let q = received::table.inner_join(message::table).inner_join(friend::table).into_boxed();

    let q = match query.limit {
      -1 => q,
      x => q.limit(x as i64),
    };

    let q = match query.filter {
      ffi::MessageFilter::New => q.filter(received::seen.eq(false)),
      ffi::MessageFilter::All => q,
      _ => {
        return Err(DbError::InvalidArgument("get_received_messages: invalid filter".to_string()))
      }
    };

    let q = match query.delivery_status {
      ffi::DeliveryStatus::Delivered => q.filter(received::delivered.eq(true)),
      ffi::DeliveryStatus::Undelivered => q.filter(received::delivered.eq(false)),
      ffi::DeliveryStatus::All => q,
      _ => {
        return Err(DbError::InvalidArgument(
          "get_received_messages: invalid delivery status".to_string(),
        ))
      }
    };

    let q = match query.sort_by {
      ffi::SortBy::None => q,
      ffi::SortBy::ReceivedAt => q.order_by(received::received_at.desc()),
      ffi::SortBy::DeliveredAt => q.order_by(received::delivered_at.desc()),
      ffi::SortBy::SentAt => {
        return Err(DbError::InvalidArgument(
          "Cannot sort by sent_at when getting received_messages".to_string(),
        ))
      }
      _ => {
        return Err(DbError::InvalidArgument("get_received_messages: invalid sort_by".to_string()))
      }
    };

    let q = match query.after {
      0 => q,
      x => match query.sort_by {
        ffi::SortBy::None => q,
        ffi::SortBy::ReceivedAt => q.filter(received::received_at.gt(x)),
        ffi::SortBy::DeliveredAt => q.filter(received::delivered_at.gt(x)),
        ffi::SortBy::SentAt => {
          return Err(DbError::InvalidArgument(
            "Cannot sort by sent_at when getting received_messages".to_string(),
          ))
        }
        _ => {
          return Err(DbError::InvalidArgument(
            "get_received_messages: invalid sort_by".to_string(),
          ))
        }
      },
    };

    q.select((
      received::uid,
      friend::unique_name,
      friend::display_name,
      received::num_chunks,
      received::received_at,
      received::delivered,
      received::delivered_at,
      received::seen,
      message::content,
    ))
    .load::<ffi::ReceivedPlusPlus>(&mut conn)
    .map_err(|e| DbError::Unknown(format!("get_received_messages: {}", e)))
  }

  pub fn get_most_recent_received_delivered_at(&self) -> Result<i64, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::received;

    self.check_rep(&mut conn);

    let q = received::table
      .filter(received::delivered.eq(true))
      .order_by(received::delivered_at.desc())
      .limit(1);

    let res = q
      .select(received::delivered_at)
      .load::<Option<i64>>(&mut conn)
      .map_err(|e| DbError::Unknown(format!("get_most_recent_received_delivered_at: {}", e)))?;

    match res.len() {
      0 => Ok(0),
      _ => match res[0] {
        Some(x) => Ok(x),
        None => Err(DbError::Unknown("get_most_recent_received_delivered_at: None".to_string())),
      },
    }
  }

  pub fn get_sent_messages(
    &self,
    query: ffi::MessageQuery,
  ) -> Result<Vec<ffi::SentPlusPlus>, DbError> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);

    use crate::schema::friend;
    use crate::schema::message;
    use crate::schema::sent;

    let q = sent::table.inner_join(message::table).inner_join(friend::table).into_boxed();

    let q = match query.limit {
      -1 => q,
      x => q.limit(x as i64),
    };

    let q = match query.filter {
      ffi::MessageFilter::All => q,
      _ => {
        return Err(DbError::InvalidArgument("get_sent_messages_query: invalid filter".to_string()))
      }
    };

    let q = match query.delivery_status {
      ffi::DeliveryStatus::Delivered => q.filter(sent::delivered.eq(true)),
      ffi::DeliveryStatus::Undelivered => q.filter(sent::delivered.eq(false)),
      ffi::DeliveryStatus::All => q,
      _ => {
        return Err(DbError::InvalidArgument(
          "get_sent_messages_query: invalid delivery status".to_string(),
        ))
      }
    };

    let q = match query.sort_by {
      ffi::SortBy::None => q,
      ffi::SortBy::SentAt => q.order_by(sent::sent_at.desc()),
      ffi::SortBy::DeliveredAt => q.order_by(sent::delivered_at.desc()),
      ffi::SortBy::ReceivedAt => {
        return Err(DbError::InvalidArgument(
          "Cannot sort by received_at when getting sent messages".to_string(),
        ))
      }
      _ => {
        return Err(DbError::InvalidArgument(
          "get_sent_messages_query: invalid sort_by".to_string(),
        ))
      }
    };

    let q = match query.after {
      0 => q,
      x => match query.sort_by {
        ffi::SortBy::None => q,
        ffi::SortBy::SentAt => q.filter(sent::sent_at.gt(x)),
        ffi::SortBy::DeliveredAt => q.filter(sent::delivered_at.gt(x)),
        ffi::SortBy::ReceivedAt => {
          return Err(DbError::InvalidArgument(
            "Cannot sort by received_at when getting sent_messages".to_string(),
          ))
        }
        _ => {
          return Err(DbError::InvalidArgument(
            "get_sent_messages_query: invalid sort_by".to_string(),
          ))
        }
      },
    };

    q.select((
      sent::uid,
      friend::unique_name,
      friend::display_name,
      sent::num_chunks,
      sent::sent_at,
      sent::delivered,
      sent::delivered_at,
      message::content,
    ))
    .load::<ffi::SentPlusPlus>(&mut conn)
    .map_err(|e| DbError::Unknown(format!("get_sent_messages: {}", e)))
  }

  pub fn get_draft_messages(
    &self,
    query: ffi::MessageQuery,
  ) -> Result<Vec<ffi::DraftPlusPlus>, DbError> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);

    use crate::schema::draft;
    use crate::schema::friend;
    use crate::schema::message;

    let q = draft::table.inner_join(message::table).inner_join(friend::table).into_boxed();

    let q = match query.limit {
      -1 => q,
      x => q.limit(x as i64),
    };

    let q = match query.filter {
      ffi::MessageFilter::All => q,
      _ => return Err(DbError::InvalidArgument("get_draft_messages: invalid filter".to_string())),
    };

    let q = match query.delivery_status {
      ffi::DeliveryStatus::All => q,
      _ => {
        return Err(DbError::InvalidArgument(
          "get_draft_messages: invalid delivery status".to_string(),
        ))
      }
    };

    let q = match query.sort_by {
      ffi::SortBy::None => q,
      _ => return Err(DbError::InvalidArgument("get_draft_messages: invalid sort_by".to_string())),
    };

    q.select((draft::uid, friend::unique_name, friend::display_name, message::content))
      .load::<ffi::DraftPlusPlus>(&mut conn)
      .map_err(|e| DbError::Unknown(format!("get_draft_messages: {}", e)))
  }

  pub fn mark_message_as_seen(&self, uid: i32) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);
    use crate::schema::received;

    let r = diesel::update(received::table.find(uid))
      .set(received::seen.eq(true))
      .returning(received::uid)
      .get_result::<i32>(&mut conn);

    self.check_rep(&mut conn);

    match r {
      Ok(_) => Ok(()),
      Err(e) => {
        Err(DbError::Unknown(format!("mark_message_as_seen, no message with that uid: {}", e)))
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  //|||                                                                    |||
  //|||                        INVITATION METHODS                          |||
  //|||                                                                    |||
  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////

  pub fn has_space_for_async_invitations(&self) -> Result<bool, DbError> {
    // return true iff no async friend requests are in the database, because we only allow 1 outgoing right now
    // TODO: update the limit to allow more outgoing async requests
    if let Ok(f) = self.get_outgoing_async_invitations() {
      if f.len() == 0 {
        return Ok(true);
      } else {
        return Ok(false);
      }
    } else {
      return Err(DbError::Unknown("failed to get outgoing async friend requests".to_string()));
    }
  }

  fn get_public_id(&self, conn: &mut SqliteConnection) -> Result<String, diesel::result::Error> {
    use crate::schema::registration;
    let q = registration::table.select(registration::public_id);
    q.first::<String>(conn)
  }

  fn can_add_friend(
    &self,
    conn: &mut SqliteConnection,
    unique_name: &str,
    kx_public_key: Vec<u8>,
    max_friends: i32,
  ) -> Result<bool, diesel::result::Error> {
    use crate::schema::complete_friend;
    use crate::schema::friend;
    use crate::schema::outgoing_async_invitation;
    use crate::schema::outgoing_sync_invitation;
    conn.transaction(|conn_b| {
      // check if a friend with this name already exists
      let count = friend::table
        .filter(friend::unique_name.eq(unique_name))
        .count()
        .get_result::<i64>(conn_b)?;
      if count > 0 {
        return Ok(false);
      }
      // check that we have don't have too many friends
      let count = friend::table.count().get_result::<i64>(conn_b)?;
      if count >= max_friends.into() {
        return Ok(false);
      }
      // check that no friend exists with the same kx_public_key
      let count_kx = complete_friend::table
        .filter(complete_friend::kx_public_key.eq(kx_public_key.clone()))
        .count()
        .get_result::<i64>(conn_b)?;
      if count_kx > 0 {
        return Ok(false);
      }
      let count_kx2 = outgoing_async_invitation::table
        .filter(outgoing_async_invitation::kx_public_key.eq(kx_public_key.clone()))
        .count()
        .get_result::<i64>(conn_b)?;
      if count_kx2 > 0 {
        return Ok(false);
      }
      let count_kx3 = outgoing_sync_invitation::table
        .filter(outgoing_sync_invitation::kx_public_key.eq(kx_public_key.clone()))
        .count()
        .get_result::<i64>(conn_b)?;
      if count_kx3 > 0 {
        return Ok(false);
      }
      Ok(true)
    })
  }

  fn create_transmission_record(
    &self,
    conn: &mut SqliteConnection,
    friend_uid: i32,
    read_index: i32,
    read_key: Vec<u8>,
    write_key: Vec<u8>,
    max_friends: i32,
  ) -> Result<(), anyhow::Error> {
    use crate::schema::friend;
    use crate::schema::transmission;

    conn.transaction::<_, anyhow::Error, _>(|conn_b| {
      let ack_indices = transmission::table
        .inner_join(friend::table)
        .filter(friend::deleted.eq(false))
        .select(transmission::ack_index)
        .load::<i32>(conn_b)?;
      let mut possible_ack_indices = Vec::<i32>::new();
      for i in 0..max_friends {
        if !ack_indices.contains(&i) {
          possible_ack_indices.push(i);
        }
      }
      use rand::seq::SliceRandom;
      let ack_index_opt = possible_ack_indices.choose(&mut rand::thread_rng());
      let ack_index = ack_index_opt.ok_or(diesel::result::Error::RollbackTransaction)
                            .map_err(|e| {
                            anyhow::Error::new(e).context("failed to choose ack index")
      })?;
      diesel::insert_into(transmission::table)
        .values((
          transmission::friend_uid.eq(friend_uid),
          transmission::read_index.eq(read_index),
          transmission::read_key.eq(read_key),
          transmission::write_key.eq(write_key),
          transmission::ack_index.eq(ack_index),
          transmission::sent_acked_seqnum.eq(0),
          transmission::received_seqnum.eq(0),
        ))
        .execute(conn_b).context("Fail to create transmission record")?;
        Ok(())
    })
  }

  pub fn add_outgoing_sync_invitation(
    &self,
    unique_name: &str,
    display_name: &str,
    story: &str,
    kx_public_key: Vec<u8>,
    read_index: i32,
    read_key: Vec<u8>,
    write_key: Vec<u8>,
    max_friends: i32,
  ) -> Result<ffi::Friend, anyhow::Error> {
    // 1. Verify that the user has space for another friend
    // 2. Verify no other friend with same unique_name
    // 3. Get public ID and chunk sequence number
    // 4. Insert friend into database
    // 5. Insert outgoing invitation into database
    // 6. Insert outgoing chunk into database
    // 7. Insert transmission information into database
    // that's it :)
    let mut conn = self.connect()?;
    use crate::schema::friend;
    use crate::schema::outgoing_chunk;
    use crate::schema::outgoing_sync_invitation;

    let friend_fragment = ffi::FriendFragment {
      unique_name: unique_name.to_string(),
      display_name: display_name.to_string(),
      invitation_progress: ffi::InvitationProgress::OutgoingSync,
      deleted: false,
    };
    let r = conn.transaction::<_, anyhow::Error, _>(|conn_b| {
      let can_add = self.can_add_friend(conn_b, unique_name, kx_public_key.clone(), max_friends)?;
      if !can_add {
        return Err(DbError::InvalidArgument(
          "add_outgoing_sync_invitation, cannot add friend for whatever reason".to_string(),
        ))
        .map_err(|e| e.into());
      }

      let friend = diesel::insert_into(friend::table)
        .values(&friend_fragment)
        .returning((
          friend::uid,
          friend::unique_name,
          friend::display_name,
          friend::invitation_progress,
          friend::deleted,
        ))
        .get_result::<ffi::Friend>(conn_b).context("Fail to insert friend into friend::table")?;

      diesel::insert_into(outgoing_sync_invitation::table)
        .values((
          outgoing_sync_invitation::friend_uid.eq(friend.uid),
          outgoing_sync_invitation::story.eq(story.to_string()),
          outgoing_sync_invitation::kx_public_key.eq(kx_public_key.clone()),
          outgoing_sync_invitation::sent_at.eq(util::unix_micros_now()),
        ))
        .execute(conn_b).context("Fail to insert into outgoing_sync_invitation::table")?;

        self.create_transmission_record(
          conn_b,
          friend.uid,
          read_index,
          read_key,
          write_key,
          max_friends,
        )?;

      let my_public_id = self.get_public_id(conn_b)?;

      let new_seqnum = self.get_seqnum_for_new_chunk(conn_b, friend.uid)?;

      diesel::insert_into(outgoing_chunk::table)
        .values((
          outgoing_chunk::to_friend.eq(friend.uid),
          outgoing_chunk::sequence_number.eq(new_seqnum),
          outgoing_chunk::chunks_start_sequence_number.eq(new_seqnum),
          outgoing_chunk::message_uid.eq::<Option<i32>>(None), //waived
          outgoing_chunk::content.eq(my_public_id), // the content is public_id
          outgoing_chunk::system.eq(true),
          outgoing_chunk::system_message.eq(ffi::SystemMessage::OutgoingInvitation),
        ))
        .execute(conn_b).context("Fail to insert into outgoing_chunk::table")?;

      Ok(friend)
    });

    r
  }

  pub fn add_outgoing_async_invitation(
    &self,
    unique_name: &str,
    display_name: &str,
    public_id: &str,
    invitation_public_key: Vec<u8>,
    kx_public_key: Vec<u8>,
    message: &str,
    read_index: i32,
    read_key: Vec<u8>,
    write_key: Vec<u8>,
    max_friends: i32,
  ) -> Result<ffi::Friend, anyhow::Error> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);

    use crate::schema::friend;
    use crate::schema::incoming_invitation;
    use crate::schema::outgoing_async_invitation;
    use crate::schema::outgoing_chunk;

    let friend_fragment = ffi::FriendFragment {
      unique_name: unique_name.to_string(),
      display_name: display_name.to_string(),
      invitation_progress: ffi::InvitationProgress::OutgoingAsync,
      deleted: false,
    };
    let r = conn.transaction::<_, anyhow::Error, _>(|conn_b| {
      let has_space = self
        .has_space_for_async_invitations()?;
      if !has_space {
        return Err(DbError::ResourceExhausted(format!(
          "add_outgoing_async_invitation, too many async invitations at the same time(currently max is 1)"
        ))).map_err(|e| e.into());
      }
      let can_add = self.can_add_friend(conn_b, unique_name, kx_public_key.clone(), max_friends)?;
      if !can_add {
        return Err(DbError::ResourceExhausted(format!(
          "add_outgoing_async_invitation, cannot add friend because limit number of friend reached."
        ))).map_err(|e| e.into());
      }
      // if there is an incoming invitation here, we reject the new outgoing async invitation
      // and tell the user to do just accept the invitation
      let incoming_invitation_count =
        incoming_invitation::table.find(public_id).count().get_result::<i64>(conn_b)?;
      if incoming_invitation_count > 0 {
        return Err(DbError::ResourceExhausted(format!(
          "add_outgoing_async_invitation, there is an incoming invitation for this public_id. please accept it"
        ))).map_err(|e| e.into());
      }
      let friend = diesel::insert_into(friend::table)
        .values(&friend_fragment)
        .returning((
          friend::uid,
          friend::unique_name,
          friend::display_name,
          friend::invitation_progress,
          friend::deleted,
        ))
        .get_result::<ffi::Friend>(conn_b).context("add_outgoing_async_invitation, failed to insert friend into friend::table")?;

      diesel::insert_into(outgoing_async_invitation::table)
        .values((
          outgoing_async_invitation::friend_uid.eq(friend.uid),
          outgoing_async_invitation::public_id.eq(public_id.to_string()),
          outgoing_async_invitation::invitation_public_key.eq(invitation_public_key),
          outgoing_async_invitation::kx_public_key.eq(kx_public_key.clone()),
          outgoing_async_invitation::message.eq(message.to_string()),
          outgoing_async_invitation::sent_at.eq(util::unix_micros_now()),
        ))
        .execute(conn_b).context("add_outgoing_async_invitation, failed to insert friend into outgoing_async_invitation::table")?;

      self.create_transmission_record(
        conn_b,
        friend.uid,
        read_index,
        read_key,
        write_key,
        max_friends,
      )?;

      let my_public_id = self.get_public_id(conn_b).context("add_outgoing_async_invitation, failed to get public_id")?;

      let new_seqnum = self.get_seqnum_for_new_chunk(conn_b, friend.uid).context("add_outgoing_async_invitation, failed to get new_seqnum")?;

      diesel::insert_into(outgoing_chunk::table)
        .values((
          outgoing_chunk::to_friend.eq(friend.uid),
          outgoing_chunk::sequence_number.eq(new_seqnum),
          outgoing_chunk::chunks_start_sequence_number.eq(new_seqnum),
          outgoing_chunk::message_uid.eq::<Option<i32>>(None), // message UID should be null
          outgoing_chunk::content.eq(my_public_id), // the content is public_id
          outgoing_chunk::system.eq(true),
          outgoing_chunk::system_message.eq(ffi::SystemMessage::OutgoingInvitation),
        ))
        .execute(conn_b).context("add_outgoing_async_invitation, failed to insert friend into outgoing_chunk::table")?;

      Ok(friend)
    });

    self.check_rep(&mut conn);

    r
  }

  fn complete_outgoing_sync_friend(
    &self,
    conn: &mut SqliteConnection,
    friend_uid: i32,
    public_id: String,
    invitation_public_key: Vec<u8>,
  ) -> Result<(), diesel::result::Error> {
    // make the friend >
    // create a complete_friend and remove a sync_outgoing_invitation
    use crate::schema::complete_friend;
    use crate::schema::friend;
    use crate::schema::outgoing_sync_invitation;

    conn.transaction(|conn_b| {
      diesel::update(friend::table.find(friend_uid))
        .set(friend::invitation_progress.eq(ffi::InvitationProgress::Complete))
        .execute(conn_b)?;

      let kx_public_key = outgoing_sync_invitation::table
        .find(friend_uid)
        .select(outgoing_sync_invitation::kx_public_key)
        .first::<Vec<u8>>(conn_b)?;

      diesel::delete(outgoing_sync_invitation::table.find(friend_uid)).execute(conn_b)?;

      diesel::insert_into(complete_friend::table)
        .values((
          complete_friend::friend_uid.eq(friend_uid),
          complete_friend::public_id.eq(public_id),
          complete_friend::invitation_public_key.eq(invitation_public_key),
          complete_friend::kx_public_key.eq(kx_public_key),
          complete_friend::completed_at.eq(util::unix_micros_now()),
        ))
        .execute(conn_b)?;

      Ok(())
    })
  }
  fn complete_outgoing_async_friend(
    &self,
    conn: &mut SqliteConnection,
    friend_uid: i32,
  ) -> Result<(), diesel::result::Error> {
    use crate::schema::complete_friend;
    use crate::schema::friend;
    use crate::schema::message;
    use crate::schema::outgoing_async_invitation;
    use crate::schema::sent;
    // 1. make the friend complete
    // 2. create a complete_friend and remove a async_outgoing_invitation
    // 3. create a message for the original outgoing async message
    conn.transaction(|conn_b| {
      diesel::update(friend::table.find(friend_uid))
        .set(friend::invitation_progress.eq(ffi::InvitationProgress::Complete))
        .execute(conn_b)?;

      let async_invitation = outgoing_async_invitation::table
        .find(friend_uid)
        .select((
          outgoing_async_invitation::friend_uid,
          outgoing_async_invitation::public_id,
          outgoing_async_invitation::invitation_public_key,
          outgoing_async_invitation::kx_public_key,
          outgoing_async_invitation::message,
          outgoing_async_invitation::sent_at,
        ))
        .first::<ffi::JustOutgoingAsyncInvitation>(conn_b)?;

      diesel::delete(outgoing_async_invitation::table.find(friend_uid)).execute(conn_b)?;

      diesel::insert_into(complete_friend::table)
        .values((
          complete_friend::friend_uid.eq(friend_uid),
          complete_friend::public_id.eq(async_invitation.public_id),
          complete_friend::invitation_public_key.eq(async_invitation.invitation_public_key),
          complete_friend::kx_public_key.eq(async_invitation.kx_public_key),
          complete_friend::completed_at.eq(util::unix_micros_now()),
        ))
        .execute(conn_b)?;

      // finally, create a message
      let message_uid = diesel::insert_into(message::table)
        .values((message::content.eq(async_invitation.message),))
        .returning(message::uid)
        .get_result::<i32>(conn_b)?;

      diesel::insert_into(sent::table)
        .values((
          sent::uid.eq(message_uid),
          sent::to_friend.eq(friend_uid),
          sent::num_chunks.eq(1),
          sent::sent_at.eq(async_invitation.sent_at),
          sent::delivered.eq(true),
          sent::delivered_at.eq(util::unix_micros_now()),
        ))
        .execute(conn_b)?;

      Ok(())
    })
  }

  pub fn add_incoming_async_invitation(
    &self,
    public_id: &str,
    message: &str,
  ) -> Result<(), DbError> {
    // It is important to define the behavior of this function in the case of
    // duplicate requests. i.e. when a friend (request) with the same public key
    // is already in the database. Here's the definition for now.
    // 2. If the friend is marked as accepted, then we ignore the request.
    // 3. If the friend is marked as incoming, then we update the message.
    // 4. If the friend is marked as outgoing async, then we approve this request.
    // (if async outgoing, we just won't know, so we cannot take any special action)
    // immediately.

    let mut conn = self.connect()?;

    self.check_rep(&mut conn);

    use crate::schema::complete_friend;
    use crate::schema::incoming_invitation;
    use crate::schema::message;
    use crate::schema::outgoing_async_invitation;
    use crate::schema::received;

    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      // check if the invitation already exists. if so, update the message, only.
      let public_id_count =
        incoming_invitation::table.find(public_id.to_string()).count().get_result::<i64>(conn_b)?;
      if public_id_count > 0 {
        diesel::update(incoming_invitation::table.find(public_id.to_string()))
          .set((
            incoming_invitation::message.eq(message.to_string()),
            incoming_invitation::received_at.eq(util::unix_micros_now()),
          ))
          .execute(conn_b)?;
        return Ok(());
      }

      // if this is already an accepted friend, we ignore
      let completed_friend_count = complete_friend::table
        .filter(complete_friend::public_id.eq(public_id.to_string()))
        .count()
        .get_result::<i64>(conn_b)?;
      if completed_friend_count > 0 {
        return Ok(());
      }

      // if this is an outgoing async friend, we make it a real friend!
      let outgoing_async_uids = outgoing_async_invitation::table
        .filter(outgoing_async_invitation::public_id.eq(public_id.to_string()))
        .select(outgoing_async_invitation::friend_uid)
        .load::<i32>(conn_b)?;
      if outgoing_async_uids.len() > 0 {
        let friend_uid = outgoing_async_uids[0];
        self.complete_outgoing_async_friend(conn_b, friend_uid)?;
        // add the incoming message as a real message! in the received table
        let message_uid = diesel::insert_into(message::table)
          .values((message::content.eq(message),))
          .returning(message::uid)
          .get_result::<i32>(conn_b)?;

        diesel::insert_into(received::table)
          .values((
            received::uid.eq(message_uid),
            received::from_friend.eq(friend_uid),
            received::num_chunks.eq(1),
            received::received_at.eq(util::unix_micros_now()),
            received::delivered.eq(true),
            received::delivered_at.eq(util::unix_micros_now()),
            received::seen.eq(false),
          ))
          .execute(conn_b)?;
        return Ok(());
      }

      // the final case is that we just create a new incoming invitation
      diesel::insert_into(incoming_invitation::table)
        .values((
          incoming_invitation::public_id.eq(public_id.to_string()),
          incoming_invitation::message.eq(message.to_string()),
          incoming_invitation::received_at.eq(util::unix_micros_now()),
        ))
        .execute(conn_b)?;

      Ok(())
    });

    self.check_rep(&mut conn);

    match r {
      Ok(_) => Ok(()),
      Err(e) => Err(DbError::Unknown(format!("add_incoming_async_invitations: {}", e))),
    }
  }

  pub fn get_outgoing_sync_invitations(&self) -> Result<Vec<ffi::OutgoingSyncInvitation>, DbError> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);
    use crate::schema::friend;
    use crate::schema::outgoing_sync_invitation;

    friend::table
      .filter(friend::deleted.eq(false))
      .filter(friend::invitation_progress.eq(ffi::InvitationProgress::OutgoingSync))
      .inner_join(outgoing_sync_invitation::table)
      .select((
        friend::uid,
        friend::unique_name,
        friend::display_name,
        friend::invitation_progress,
        outgoing_sync_invitation::story,
        outgoing_sync_invitation::sent_at,
      ))
      .load::<ffi::OutgoingSyncInvitation>(&mut conn)
      .map_err(|e| DbError::Unknown(format!("get_outgoing_sync_invitations: {}", e)))
  }

  pub fn get_outgoing_async_invitations(
    &self,
  ) -> Result<Vec<ffi::OutgoingAsyncInvitation>, DbError> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);
    use crate::schema::friend;
    use crate::schema::outgoing_async_invitation;

    friend::table
      .filter(friend::deleted.eq(false))
      .filter(friend::invitation_progress.eq(ffi::InvitationProgress::OutgoingAsync))
      .inner_join(outgoing_async_invitation::table)
      .select((
        friend::uid,
        friend::unique_name,
        friend::display_name,
        friend::invitation_progress,
        outgoing_async_invitation::public_id,
        outgoing_async_invitation::invitation_public_key,
        outgoing_async_invitation::kx_public_key,
        outgoing_async_invitation::message,
        outgoing_async_invitation::sent_at,
      ))
      .load::<ffi::OutgoingAsyncInvitation>(&mut conn)
      .map_err(|e| DbError::Unknown(format!("get_outgoing_async_invitations: {}", e)))
  }

  pub fn get_incoming_invitations(&self) -> Result<Vec<ffi::IncomingInvitation>, DbError> {
    let mut conn = self.connect()?; // if error then crash function
    self.check_rep(&mut conn);
    use crate::schema::incoming_invitation;

    incoming_invitation::table
      .select((
        incoming_invitation::public_id,
        incoming_invitation::message,
        incoming_invitation::received_at,
      ))
      .load::<ffi::IncomingInvitation>(&mut conn)
      .map_err(|e| DbError::Unknown(format!("get_incoming_invitations: {}", e)))
  }

  pub fn accept_incoming_invitation(
    &self,
    public_id: &str,
    unique_name: &str,
    display_name: &str,
    invitation_public_key: Vec<u8>,
    kx_public_key: Vec<u8>,
    read_index: i32,
    read_key: Vec<u8>,
    write_key: Vec<u8>,
    max_friends: i32,
  ) -> Result<(), anyhow::Error> {
    // // This function is called when the user accepts a friend request.
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);

    use crate::schema::complete_friend;
    use crate::schema::friend;
    use crate::schema::incoming_invitation;
    use crate::schema::message;
    use crate::schema::received;

    let friend_fragment = ffi::FriendFragment {
      unique_name: unique_name.to_string(),
      display_name: display_name.to_string(),
      invitation_progress: ffi::InvitationProgress::Complete,
      deleted: false,
    };
    // we change the progress field to Complete, meaning that the friend is approved
    let r = conn
      .transaction::<_, anyhow::Error, _>(|conn_b| {
        let can_add =
          self.can_add_friend(conn_b, unique_name, kx_public_key.clone(), max_friends)?;
        if !can_add {
          // return an anyhow error
          anyhow::anyhow!("no free ack index");
        }
        // we create a new friend
        let friend = diesel::insert_into(friend::table)
          .values(&friend_fragment)
          .returning((
            friend::uid,
            friend::unique_name,
            friend::display_name,
            friend::invitation_progress,
            friend::deleted,
          ))
          .get_result::<ffi::Friend>(conn_b)?;

        let inc_invitation = incoming_invitation::table
          .filter(incoming_invitation::public_id.eq(public_id.to_string()))
          .get_result::<ffi::IncomingInvitation>(conn_b)?;

        diesel::delete(incoming_invitation::table.find(public_id)).execute(conn_b)?;

        diesel::insert_into(complete_friend::table)
          .values((
            complete_friend::friend_uid.eq(friend.uid),
            complete_friend::public_id.eq(public_id),
            complete_friend::invitation_public_key.eq(invitation_public_key),
            complete_friend::kx_public_key.eq(kx_public_key),
            complete_friend::completed_at.eq(util::unix_micros_now()),
          ))
          .execute(conn_b)?;

        // finally, create a message
        let message_uid = diesel::insert_into(message::table)
          .values((message::content.eq(inc_invitation.message),))
          .returning(message::uid)
          .get_result::<i32>(conn_b)?;

        diesel::insert_into(received::table)
          .values((
            received::uid.eq(message_uid),
            received::from_friend.eq(friend.uid),
            received::num_chunks.eq(1),
            received::received_at.eq(inc_invitation.received_at),
            received::delivered.eq(true),
            received::delivered_at.eq(util::unix_micros_now()),
            received::seen.eq(false),
          ))
          .execute(conn_b)?;

        self.create_transmission_record(
          conn_b,
          friend.uid,
          read_index,
          read_key,
          write_key,
          max_friends,
        )?;

        Ok(())
      });

    self.check_rep(&mut conn);

    r
  }

  pub fn deny_incoming_invitation(&self, public_id: &str) -> Result<(), DbError> {
    // This function is called when the user rejects a friend request.
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);
    use crate::schema::incoming_invitation;

    // delete public_id from incoming_invitation
    let r = diesel::delete(
      incoming_invitation::table.filter(incoming_invitation::public_id.eq(public_id)),
    )
    .execute(&mut conn);

    self.check_rep(&mut conn);

    match r {
      Ok(_) => Ok(()),
      Err(e) => Err(DbError::Unknown(format!("deny_incoming_invitation: {}", e))),
    }
  }

  pub fn receive_invitation_system_message(
    &self,
    from_friend: i32,
    sequence_number: i32,
    public_id: &str,
    public_id_claimed_kx_public_key: Vec<u8>,
    public_id_claimed_invitation_public_key: Vec<u8>,
  ) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    self.check_rep(&mut conn);
    use crate::schema::friend;
    use crate::schema::outgoing_async_invitation;
    use crate::schema::outgoing_sync_invitation;

    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let chunk_status = self.update_sequence_number(conn_b, from_friend, sequence_number)?;
      if chunk_status == ffi::ReceiveChunkStatus::OldChunk {
        return Ok(());
      }

      let friend_status = friend::table
        .find(from_friend)
        .select(friend::invitation_progress)
        .get_result::<ffi::InvitationProgress>(conn_b)?;

      match friend_status {
        ffi::InvitationProgress::OutgoingSync => {
          // verify that the kx_public_key is correct
          let friend_kx_public_key = outgoing_sync_invitation::table
            .find(from_friend)
            .select(outgoing_sync_invitation::kx_public_key)
            .get_result::<Vec<u8>>(conn_b)?;
          if friend_kx_public_key != public_id_claimed_kx_public_key {
            // something fishy is happening.... they are trying to deceive us!
            return Err(diesel::result::Error::RollbackTransaction);
          }
          self.complete_outgoing_sync_friend(
            conn_b,
            from_friend,
            public_id.to_string(),
            public_id_claimed_invitation_public_key,
          )?;
        }
        ffi::InvitationProgress::OutgoingAsync => {
          // verify that the public_id is correct
          let friend_public_id = outgoing_async_invitation::table
            .find(from_friend)
            .select(outgoing_async_invitation::public_id)
            .get_result::<String>(conn_b)?;
          if friend_public_id != public_id {
            return Err(diesel::result::Error::RollbackTransaction);
          }
          // make it a real friend!
          self.complete_outgoing_async_friend(conn_b, from_friend)?;
        }
        ffi::InvitationProgress::Complete => (),
        _ => {
          return Err(diesel::result::Error::RollbackTransaction);
        }
      }

      Ok(())
    });
    self.check_rep(&mut conn);

    match r {
      Ok(_) => Ok(()),
      Err(e) => Err(DbError::Unknown(format!("receive_invitation_system_message: {}", e))),
    }
  }
}
