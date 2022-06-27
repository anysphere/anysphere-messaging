use diesel::prelude::*;

use std::{error::Error, fmt};

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
pub const CONTROL_MESSAGE_OUTGOING_FRIEND_REQUEST = 0;

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

///
/// Source of truth is in the migrations folder. Schema.rs is generated from them.
/// This right here is just a query interface. It will not correspond exactly. It should not.
///
#[cxx::bridge(namespace = "db")]
pub mod ffi {

  //
  // WARNING: Diesel checks types. That's awesome. But IT DOES NOT CHECK THE ORDER.
  // For example: in Friend, the order of enabled and deleted is not checked,
  // so we need to make sure that the order here matches the order in schema.rs.
  //
  // NEVER EVER CHANGE THE ORDER OF THE FIELDS HERE WITHOUT LOOKING AT ALL QUERIES WHERE
  // THEY ARE USED. @code_review
  //
  // TODO: try to write a macro for enforcing this in code.
  //

  #[derive(Queryable)]
  struct Friend {
    pub uid: i32,
    pub unique_name: String,
    pub display_name: String,
    pub enabled: bool,
    pub deleted: bool,
  }
  #[derive(Insertable)]
  #[diesel(table_name = crate::schema::friend)]
  struct FriendFragment {
    pub unique_name: String,
    pub display_name: String,
    pub enabled: bool,
    pub deleted: bool,
  }

  #[derive(Queryable, Insertable)]
  #[diesel(table_name = crate::schema::address)]
  struct Address {
    pub uid: i32,
    pub read_index: i32,
    pub ack_index: i32,
    pub read_key: Vec<u8>,
    pub write_key: Vec<u8>,
  }
  struct AddAddress {
    pub unique_name: String,
    pub read_index: i32,
    pub read_key: Vec<u8>,
    pub write_key: Vec<u8>,
  }

  #[derive(Queryable, Insertable)]
  #[diesel(table_name = crate::schema::status)]
  struct Status {
    pub uid: i32,
    pub sent_acked_seqnum: i32,
    pub received_seqnum: i32,
  }

  #[derive(Queryable)]
  struct Registration {
    pub uid: i32,
    pub public_key: Vec<u8>,
    pub private_key: Vec<u8>,
    pub allocation: i32,
    pub pir_secret_key: Vec<u8>,
    pub pir_galois_key: Vec<u8>,
    pub authentication_token: String,
  }
  #[derive(Insertable)]
  #[diesel(table_name = crate::schema::registration)]
  struct RegistrationFragment {
    pub public_key: Vec<u8>,
    pub private_key: Vec<u8>,
    pub allocation: i32,
    pub pir_secret_key: Vec<u8>,
    pub pir_galois_key: Vec<u8>,
    pub authentication_token: String,
  }
  #[derive(Queryable)]
  struct SendInfo {
    pub allocation: i32,
    pub authentication_token: String,
  }
  #[derive(Queryable)]
  struct SmallRegistrationFragment {
    pub uid: i32,
    pub public_key: Vec<u8>,
    pub private_key: Vec<u8>,
    pub allocation: i32,
    pub authentication_token: String,
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

  #[derive(Queryable)]
  struct OutgoingChunkPlusPlus {
    pub to_friend: i32,
    pub sequence_number: i32,
    pub chunks_start_sequence_number: i32,
    pub message_uid: i32,
    pub content: String,
    pub write_key: Vec<u8>,
    pub num_chunks: i32,
    pub control: bool,
    pub control_message: i32,
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
    fn get_friends(&self) -> Result<Vec<Friend>>;
    // fails if a friend with unique_name already exists
    fn create_friend(
      &self,
      unique_name: &str,
      display_name: &str,
      max_friends: i32,
    ) -> Result<Friend>;
    // adds a friend address and also makes the friend enabled
    fn add_friend_address(&self, add_address: AddAddress, max_friends: i32) -> Result<()>;
    fn delete_friend(&self, unique_name: &str) -> Result<()>;
    // returns address iff enabled && !deleted
    fn get_friend_address(&self, uid: i32) -> Result<Address>;
    // fails if no such friend exists
    fn get_random_enabled_friend_address_excluding(&self, uids: Vec<i32>) -> Result<Address>;

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
    // receive the control message telling us to add the friend
    fn receive_friend_request_control_message(
      &self,
      from_friend: i32,
      sequence_number: i32,
    ) -> Result<()>;

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

    let q = config::table.select(config::has_registered);
    let has_registered = q
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query has_registered: {}", e)))?;
    Ok(has_registered)
  }

  pub fn do_register(&self, reg: ffi::RegistrationFragment) -> Result<(), DbError> {
    let mut conn = self.connect()?;

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

    let registration = registration::table
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query registration: {}", e)))?;
    Ok(registration)
  }

  pub fn delete_registration(&self) -> Result<(), DbError> {
    let mut conn = self.connect()?;

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

    Ok(())
  }

  pub fn get_small_registration(&self) -> Result<ffi::SmallRegistrationFragment, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::registration;

    let q = registration::table.select((
      registration::uid,
      registration::public_key,
      registration::private_key,
      registration::allocation,
      registration::authentication_token,
    ));
    let registration = q
      .first::<ffi::SmallRegistrationFragment>(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query registration: {}", e)))?;
    Ok(registration)
  }

  pub fn get_pir_secret_key(&self) -> Result<Vec<u8>, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::registration;

    let q = registration::table.select(registration::pir_secret_key);
    let pir_secret_key = q
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query pir_secret_key: {}", e)))?;
    Ok(pir_secret_key)
  }

  pub fn get_send_info(&self) -> Result<ffi::SendInfo, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::registration;

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

    if let Ok(f) =
      friend::table.filter(friend::unique_name.eq(unique_name)).first::<ffi::Friend>(&mut conn)
    {
      Ok(f)
    } else {
      Err(DbError::NotFound("failed to get friend".to_string()))
    }
  }

  pub fn get_friends(&self) -> Result<Vec<ffi::Friend>, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::friend;

    if let Ok(v) = friend::table.filter(friend::deleted.eq(false)).load::<ffi::Friend>(&mut conn) {
      Ok(v)
    } else {
      Err(DbError::Unknown("failed to get friends".to_string()))
    }
  }

  pub fn create_friend(
    &self,
    unique_name: &str,
    display_name: &str,
    max_friends: i32,
  ) -> Result<ffi::Friend, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::friend;

    let f = ffi::FriendFragment {
      unique_name: unique_name.to_string(),
      display_name: display_name.to_string(),
      enabled: false,
      deleted: false,
    };
    let r = conn.transaction(|conn_b| {
      // check if a friend with this name already exists
      let count = friend::table
        .filter(friend::unique_name.eq(unique_name))
        .count()
        .get_result::<i64>(conn_b)?;
      if count > 0 {
        return Err(diesel::result::Error::RollbackTransaction);
      }
      let count = friend::table.count().get_result::<i64>(conn_b)?;
      if count >= max_friends.into() {
        return Err(diesel::result::Error::RollbackTransaction);
      }

      diesel::insert_into(friend::table).values(&f).get_result::<ffi::Friend>(conn_b)
    });

    r.map_err(|e| match e {
      diesel::result::Error::RollbackTransaction => {
        DbError::AlreadyExists("friend already exists, or too many friends".to_string())
      }
      _ => DbError::Unknown(format!("failed to insert friend: {}", e)),
    })
  }

  pub fn add_friend_address(
    &self,
    add_address: ffi::AddAddress,
    max_friends: i32,
  ) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    use crate::schema::address;
    use crate::schema::friend;
    use crate::schema::status;

    // transaction because we need to pick a new ack_index
    conn
      .transaction(|conn_b| {
        let uid = friend::table
          .filter(friend::unique_name.eq(add_address.unique_name))
          .select(friend::uid)
          .first::<i32>(conn_b)?;

        let ack_indices = address::table
          .inner_join(friend::table)
          .filter(friend::deleted.eq(false))
          .select(address::ack_index)
          .load::<i32>(conn_b)?;
        let mut possible_ack_indices = Vec::<i32>::new();
        for i in 0..max_friends {
          if !ack_indices.contains(&i) {
            possible_ack_indices.push(i);
          }
        }
        use rand::seq::SliceRandom;
        let ack_index_opt = possible_ack_indices.choose(&mut rand::thread_rng());
        let ack_index = ack_index_opt.ok_or(diesel::result::Error::RollbackTransaction)?;
        let address = ffi::Address {
          uid,
          read_index: add_address.read_index,
          ack_index: *ack_index,
          read_key: add_address.read_key,
          write_key: add_address.write_key,
        };
        diesel::insert_into(address::table).values(&address).execute(conn_b)?;

        diesel::update(friend::table.find(uid)).set(friend::enabled.eq(true)).execute(conn_b)?;

        let status = ffi::Status { uid, sent_acked_seqnum: 0, received_seqnum: 0 };
        diesel::insert_into(status::table).values(&status).execute(conn_b)?;

        Ok(())
      })
      .map_err(|e| match e {
        diesel::result::Error::RollbackTransaction => {
          DbError::AlreadyExists("no free ack index".to_string())
        }
        _ => DbError::Unknown(format!("failed to insert address: {}", e)),
      })
  }

  pub fn delete_friend(&self, unique_name: &str) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    use crate::schema::friend;

    diesel::update(friend::table.filter(friend::unique_name.eq(unique_name)))
      .set(friend::deleted.eq(false))
      .returning(friend::uid)
      .get_result::<i32>(&mut conn)
      .map_err(|e| match e {
        diesel::result::Error::NotFound => DbError::NotFound("friend not found".to_string()),
        _ => DbError::Unknown(format!("failed to delete friend: {}", e)),
      })?;

    Ok(())
  }

  pub fn set_latency(&self, latency: i32) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    use crate::schema::config;

    let r = diesel::update(config::table).set(config::latency.eq(latency)).execute(&mut conn);

    match r {
      Ok(_) => Ok(()),
      Err(e) => Err(DbError::Unknown(format!("set_latency: {}", e))),
    }
  }

  pub fn get_latency(&self) -> Result<i32, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::config;

    let q = config::table.select(config::latency);
    let latency = q
      .first(&mut conn)
      .map_err(|e| DbError::Unknown(format!("failed to query latency: {}", e)))?;
    Ok(latency)
  }

  pub fn set_server_address(&self, server_address: &str) -> Result<(), DbError> {
    let mut conn = self.connect()?;
    use crate::schema::config;

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

    match config::table.select(config::server_address).first(&mut conn) {
      Ok(server_address) => Ok(server_address),
      Err(e) => Err(DbError::Unknown(format!("get_server_address: {}", e))),
    }
  }

  pub fn receive_ack(&self, uid: i32, ack: i32) -> Result<bool, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::outgoing_chunk;
    use crate::schema::sent;
    use crate::schema::status;

    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let old_acked_seqnum =
        status::table.find(uid).select(status::sent_acked_seqnum).first(conn_b)?;
      if ack > old_acked_seqnum {
        diesel::update(status::table.find(uid))
          .set(status::sent_acked_seqnum.eq(ack))
          .execute(conn_b)?;
        // check if there are any control messages that were ACKed
        let control_chunks = outgoing_chunk::table
          .filter(outgoing_chunk::to_friend.eq(uid))
          .filter(outgoing_chunk::sequence_number.le(ack))
          .filter(outgoing_chunk::control.eq(true))
          .select(
            outgoing_chunk::control_message
          )
          .load::<i32>(conn_b)?;
        for control_message in control_chunks {
          if control_message == CONTROL_MESSAGE_OUTGOING_FRIEND_REQUEST {
            // yay! they shall now be considered a Real Friend
            diesel::update(friend::table.find(uid))
              .set(friend::progress.eq(ACTUAL_FRIEND)
              .execute(conn_b)?;
          }
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
    from_friend: i32,
    sequence_number: i32,
  ) -> Result<ffi::ReceiveChunkStatus, diesel::result::Error> {
    use crate::schema::status;

    conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let old_seqnum =
        status::table.find(from_friend).select(status::received_seqnum).first::<i32>(conn_b)?;
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
        diesel::update(status::table.find(from_friend))
          .set(status::received_seqnum.eq(sequence_number))
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

    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let chunk_status = update_sequence_number(chunk.from_friend, chunk.sequence_number)?;
      if (chunk_status == ffi::ReceiveChunkStatus::OldChunk) {
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

    match r {
      Ok(b) => Ok(b),
      Err(e) => Err(DbError::Unknown(format!("receive_chunk: {}", e))),
    }
  }

  fn receive_friend_request_control_message(
    &self,
    from_friend: i32,
    sequence_number: i32,
  ) -> Result<(), DbError> {
    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
      let chunk_status = update_sequence_number(from_friend, sequence_number)?;
      if (chunk_status == ffi::ReceiveChunkStatus::OldChunk) {
        return Ok(chunk_status);
      }
      // move the friend to become an actual friend
      diesel::update(friend::table.find(from_friend))
        .set(friend::progress.eq(ACTUAL_FRIEND))
        .execute(conn_b)?;

      Ok(ffi::ReceiveChunkStatus::NewChunk)
    });

    match r {
      Ok(b) => Ok(),
      Err(e) => Err(DbError::Unknown(format!("receive_chunk: {}", e))),
    }
  }

  pub fn chunk_to_send(
    &self,
    uid_priority: Vec<i32>,
  ) -> Result<ffi::OutgoingChunkPlusPlus, DbError> {
    let mut conn = self.connect()?;

    use crate::schema::address;
    use crate::schema::friend;
    use crate::schema::outgoing_chunk;
    use crate::schema::sent;

    // We could do probably this in one query, by joining on the select statement
    // and then joining. Diesel doesn't typecheck this, and maybe it is unsafe, so
    // let's just do a transaction.
    let r = conn.transaction::<_, diesel::result::Error, _>(|conn_b| {
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
        return Err(diesel::result::Error::NotFound);
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
      let chunk_plusplus = outgoing_chunk::table
        .find(chosen_chunk)
        .inner_join(friend::table.inner_join(address::table))
        .inner_join(sent::table)
        .select((
          outgoing_chunk::to_friend,
          outgoing_chunk::sequence_number,
          outgoing_chunk::chunks_start_sequence_number,
          outgoing_chunk::message_uid,
          outgoing_chunk::content,
          address::write_key,
          sent::num_chunks,
          outgoing_chunk::control,
          outgoing_chunk::control_message,
        ))
        .first::<ffi::OutgoingChunkPlusPlus>(conn_b)?;
      Ok(chunk_plusplus)
    });

    match r {
      Ok(chunk_plusplus) => Ok(chunk_plusplus),
      Err(e) => Err(DbError::NotFound(format!("chunk_to_send: {}", e))),
    }
  }

  pub fn acks_to_send(&self) -> Result<Vec<ffi::OutgoingAck>, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::address;
    use crate::schema::friend;
    use crate::schema::status;
    let wide_friends = friend::table
      .filter(friend::enabled.eq(true))
      .filter(friend::deleted.eq(false))
      .inner_join(status::table)
      .inner_join(address::table);
    let q = wide_friends.select((
      friend::uid,
      status::received_seqnum,
      address::write_key,
      address::ack_index,
    ));
    let r = q.load::<ffi::OutgoingAck>(&mut conn);
    match r {
      Ok(acks) => Ok(acks),
      Err(e) => Err(DbError::Unknown(format!("acks_to_send: {}", e))),
    }
  }

  pub fn get_friend_address(&self, uid: i32) -> Result<ffi::Address, DbError> {
    let mut conn = self.connect()?;

    use crate::schema::address;

    match address::table.find(uid).first(&mut conn) {
      Ok(address) => Ok(address),
      Err(e) => Err(DbError::Unknown(format!("get_friend_address: {}", e))),
    }
  }

  pub fn get_random_enabled_friend_address_excluding(
    &self,
    uids: Vec<i32>,
  ) -> Result<ffi::Address, DbError> {
    let mut conn = self.connect()?;
    use crate::schema::address;
    use crate::schema::friend;

    // get a random friend that is not deleted excluding the ones in the uids list
    let q = friend::table.filter(friend::enabled.eq(true)).filter(friend::deleted.eq(false));

    // Inner join to get the (friend, address) pairs and then select the address.
    let q = q.inner_join(address::table).select(address::all_columns);
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

  pub fn queue_message_to_send(
    &self,
    to_unique_name: &str,
    message: &str,
    chunks: Vec<String>,
  ) -> Result<(), DbError> {
    // TODO: implement this
    // We chunk in C++ because we potentially need to do things with protobuf
    // What do we do here?
    // 1. Create a new message.
    // 2. Create a new sent message.
    // 1. Take in all chunks, create outgoing_chunks.
    let mut conn = self.connect()?;
    use crate::schema::message;
    use crate::schema::outgoing_chunk;
    use crate::schema::sent;
    use crate::schema::status;

    conn
      .transaction::<_, diesel::result::Error, _>(|conn_b| {
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

        // get the correct sequence number. This is the last sequence number + 1.
        // this is either the maximum sequence number in the table + 1,
        // or the sent_acked_seqnum + 1 if there are no outgoing chunks.
        let maybe_old_seqnum = outgoing_chunk::table
          .filter(outgoing_chunk::to_friend.eq(friend_uid))
          .select(outgoing_chunk::sequence_number)
          .order_by(outgoing_chunk::sequence_number.desc())
          .limit(1)
          .load::<i32>(conn_b)?;
        let old_seqnum = match maybe_old_seqnum.len() {
          0 => {
            status::table.find(friend_uid).select(status::sent_acked_seqnum).first::<i32>(conn_b)?
          }
          _ => maybe_old_seqnum[0],
        };
        let new_seqnum = old_seqnum + 1;

        for (i, chunk) in chunks.iter().enumerate() {
          diesel::insert_into(outgoing_chunk::table)
            .values((
              outgoing_chunk::to_friend.eq(friend_uid),
              outgoing_chunk::sequence_number.eq(new_seqnum + i as i32),
              outgoing_chunk::chunks_start_sequence_number.eq(new_seqnum),
              outgoing_chunk::message_uid.eq(message_uid),
              outgoing_chunk::content.eq(chunk),
            ))
            .execute(conn_b)?;
        }

        Ok(())
      })
      .map_err(|e| {
        DbError::Unknown(format!("queue_message_to_send: {} (maybe friend doesn't exist?)", e))
      })?;

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
    use crate::schema::received;

    let r = diesel::update(received::table.find(uid))
      .set(received::seen.eq(true))
      .returning(received::uid)
      .get_result::<i32>(&mut conn);

    match r {
      Ok(_) => Ok(()),
      Err(e) => {
        Err(DbError::Unknown(format!("mark_message_as_seen, no message with that uid: {}", e)))
      }
    }
  }
}
