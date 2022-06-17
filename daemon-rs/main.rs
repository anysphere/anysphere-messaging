#[macro_use]
extern crate diesel;
use diesel::prelude::*;
mod schema;

use std::time::SystemTime;

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("daemon-rs/main.hpp");

        fn main_cc(args: Vec<String>) -> i32;
    }
}

fn main() {
    use std::env;
    let args: Vec<String> = env::args().collect();
    ffi::main_cc(args);
}

use std::{error::Error, fmt};

#[derive(Debug)]
enum DbError {
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

// TODO(arvid): manage a connection pool for the DB here?
// i.e. pool: Box<ConnectionPool> or something
struct DB {
    address: String,
}

#[derive(Queryable, Insertable)]
struct Sent {
    pub uid: i32,
    pub to_friend: i32,
    pub num_chunks: i32,
    pub sent_at: SystemTime,
    pub delivered: bool,
    pub delivered_at: Option<SystemTime>,
}

#[derive(Queryable, Insertable)]
struct Received {
    pub uid: i32,
    pub from_friend: i32,
    pub num_chunks: i32,
    pub received_at: SystemTime,
    pub delivered: bool,
    pub delivered_at: Option<SystemTime>,
    pub seen: bool,
}

//
// Source of truth is in the migrations folder. Schema.rs is generated from them.
// This right here is just a query interface. It will not correspond exactly. It should not.
//
#[cxx::bridge(namespace = "db")]
pub mod db {

    #[derive(Queryable)]
    struct Friend {
        pub uid: i32,
        pub unique_name: String,
        pub display_name: String,
        pub enabled: bool,
        pub deleted: bool,
    }

    #[derive(Queryable)]
    struct Address {
        pub uid: i32,
        pub read_index: i32,
        pub ack_index: i32,
        pub read_key: Vec<u8>,
        pub write_key: Vec<u8>,
    }

    #[derive(Queryable)]
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

    #[derive(Queryable)]
    struct SendInfo {
        pub allocation: i32,
        pub authentication_token: String,
    }


    #[derive(Queryable, Insertable)]
    struct IncomingChunk {
        pub from_friend: i32,
        pub sequence_number: i32,
        pub chunks_start_sequence_number: i32,
        pub message_uid: i32,
        pub s: String,
    }

    struct IncomingIncomingChunk {
        pub from_friend: i32,
        pub sequence_number: i32,
        pub chunks_start_sequence_number: i32,
        pub num_chunks: i32,
        pub s: String,
    }

    #[derive(Queryable)]
    struct OutgoingChunk {
        pub to_friend: i32,
        pub sequence_number: i32,
        pub chunks_start_sequence_number: i32,
        pub message_uid: i32,
        pub message: Vec<u8>,
        pub write_key: Vec<u8>,
    }

    #[derive(Queryable)]
    struct OutgoingAck {
        pub to_friend: i32,
        pub ack: i32,
        pub write_key: Vec<u8>,
    }

    extern "Rust" {
        type DB;
        fn init(address: &str, default_server_address: &str, default_latency: i32) -> Result<Box<DB>>;

        //
        // Config
        //
        fn set_latency(&self, latency: i32, default_latency: i32) -> Result<()>;
        fn get_latency(&self) -> Result<i32>;
        fn set_server_address(&self, server_address: &str, default_server_address: &str) -> Result<()>;
        fn get_server_address(&self) -> Result<String>;

        //
        // Registration
        //
        fn has_registered(&self) -> Result<bool>;
        fn get_registration(&self) -> Result<Registration>;
        fn get_pir_secret_key(&self) -> Result<Vec<u8>>;
        fn get_send_info(&self) -> Result<SendInfo>;

        //
        // Friends
        //
        fn get_friend(&self, uid: i32) -> Result<Friend>;
        // returns address iff enabled && !deleted
        fn get_friend_address(&self, uid: i32) -> Result<Address>;
        // fails if no such friend exists
        fn get_random_enabled_friend_address_excluding(&self, uids: Vec<i32>) -> Result<Address>;

        //
        // Messages
        //
        // returns true iff the ack was novel
        fn receive_ack(&self, uid: i32, ack: i32) -> Result<bool>;
        fn receive_chunk(&self, chunk: IncomingIncomingChunk) -> Result<()>;

        // fails if there is no chunk to send
        // prioritizes by the given uid in order from first to last try
        fn chunk_to_send(&self, uid_priority: Vec<i32>) -> Result<OutgoingChunk>;
        fn acks_to_send(&self) -> Result<Vec<OutgoingAck>>;
    }

}

fn init(address: &str, default_server_address: &str, default_latency: i32) -> Result<Box<DB>, DbError> {
    let db = DB {
        address: address.to_string(),
    };
    let conn = db.connect()?;

    // TODO(arvid): run migrations

    use self::schema::config;

    let q = diesel::update(config::table.filter(config::server_address_default.eq(true))).set(config::server_address.eq(default_server_address));
    match q.execute(&conn) {
        Ok(_) => (),
        Err(e) => return Err(DbError::Unknown(format!("failed to set default server address: {}", e))),
    };

    let q = diesel::update(config::table.filter(config::latency_default.eq(true))).set(config::latency.eq(default_latency));
    match q.execute(&conn) {
        Ok(_) => (),
        Err(e) => return Err(DbError::Unknown(format!("failed to set default latency: {}", e))),
    };

    Ok(Box::new(db))
}

fn print_query<T: diesel::query_builder::QueryFragment<diesel::sqlite::Sqlite>>(q: &T) {
    println!("print_query: {}", diesel::debug_query::<diesel::sqlite::Sqlite, _>(&q));
}

impl DB {
    fn connect(&self) -> Result<SqliteConnection, DbError> {
        match SqliteConnection::establish(&self.address) {
            Ok(c) => return Ok(c),
            Err(e) => return Err(DbError::Unknown("failed to connect to database".to_string())),
        }
    }

    fn has_registered(&self) -> Result<bool, DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        let q = config::table.select(config::has_registered);
        let has_registered = q.first(&conn).map_err(|e| DbError::Unknown(format!("failed to query has_registered: {}", e)))?;
        Ok(has_registered)
    }

    fn get_registration(&self) -> Result<Registration, DbError> {
        let conn = self.connect()?;
        use self::schema::registration;

        let q = registration::table.select(registration::all_columns);
        let registration = q.first(&conn).map_err(|e| DbError::Unknown(format!("failed to query registration: {}", e)))?;
        Ok(registration)
    }

    fn get_pir_secret_key(&self) -> Result<Vec<u8>> {
        let conn = self.connect()?;
        use self::schema::registration;

        let q = registration::table.select(registration::pir_secret_key);
        let pir_secret_key = q.first(&conn).map_err(|e| DbError::Unknown(format!("failed to query pir_secret_key: {}", e)))?;
        Ok(pir_secret_key)
    }

    fn get_friend(&self, uid: i32) -> Result<db::Friend, DbError> {
        let conn = self.connect()?;
        use self::schema::friend;

        if let Ok(mut v) = friend::table.find(uid).load::<db::Friend>(&conn) {
            if v.len() == 0 {
                return Err(DbError::NotFound("friend not found".to_string()));
            }
            Ok(v.remove(0))
        } else {
            Err(DbError::Unknown("failed to get friend".to_string()))
        }
    }


    fn set_latency(&self, latency: i32, default_latency: i32) -> Result<(), DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        let r = conn.transaction::<_, diesel::result::Error, _>(|| {
            if latency == default_latency {
                diesel::update(config::table).set(config::latency_default.eq(true)).execute(&conn)?;
            } else {
                diesel::update(config::table).set(config::latency_default.eq(true)).execute(&conn)?;
            }
            diesel::update(config::table).set(config::latency.eq(latency)).execute(&conn)
        });

        match r {
            Ok(_) => Ok(()),
            Err(e) => Err(DbError::Unknown(format!("set_latency: {}", e))),
        }
    }

    fn get_latency(&self) -> Result<i32, DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        let q = config::table.select(config::latency);
        let latency = q.first(&conn).map_err(|e| DbError::Unknown(format!("failed to query latency: {}", e)))?;
        Ok(latency)
    }

    fn set_server_address(&self, server_address: &str, default_server_address: &str) -> Result<(), DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        let r = conn.transaction::<_, diesel::result::Error, _>(|| {
            if server_address == default_server_address {
                diesel::update(config::table).set(config::server_address_default.eq(true)).execute(&conn)?;
            } else {
                diesel::update(config::table).set(config::server_address_default.eq(false)).execute(&conn)?;
            }
            diesel::update(config::table).set(config::server_address.eq(server_address)).execute(&conn)
        });

        match r {
            Ok(_) => Ok(()),
            Err(e) => Err(DbError::Unknown(format!("set_server_address: {}", e))),
        }
    }

    fn get_server_address(&self) -> Result<String, DbError> {
        let conn = self.connect()?;
        use self::schema::config;

        match config::table.select(config::server_address).first(&conn) {
            Ok(s) => match s {
                Some(s) => Ok(s),
                None => Err(DbError::NotFound("server address not found".to_string())),
            },
            Err(e) => Err(DbError::Unknown(format!("get_server_address: {}", e))),
        }
    }

    fn receive_ack(&self, uid: i32, ack: i32) -> Result<(), DbError> {
        let conn = self.connect()?;
        use self::schema::status;
        use self::schema::outgoing_chunk;

        let r = conn.transaction::<_, diesel::result::Error, _>(|| {
            let old_ack = status::table.find(status::uid.eq(uid)).select(status::ack).first(&conn)?;
            if ack > old_ack {
                diesel::update(status::table.find(status::uid.eq(uid))).set(status::ack.eq(ack)).execute(&conn)?;
                // delete outgoing chunk if it exists
                diesel::delete(outgoing_chunk::table.filter((outgoing_chunk::to_friend.eq(uid), outgoing_chunk::sequence_number.eq(ack)))).execute(&conn)?;
            }
            return old_ack;
        });
        match r {
            Ok(old_ack) => {
                if ack > old_ack {
                    Ok(true)
                } else if ack == old_ack {
                    Ok(false)
                } else {
                    println!("Ack is older than old ack. This is weird, and probably indicates that the person you're talking to has done something wrong.");
                    Ok(false)
                }
            }
            Err(e) => Err(DbError::Unknown(format!("receive_ack: {}", e))),
        }
    }

    // fn receive_chunk(&self, chunk: IncomingChunk) -> Result<(), DbError> {
    //     let conn = self.connect()?;
    //     use self::schema::incoming_chunk;
    //     use self::schema::message;
    //     use self::schema::received;

    //     let r = conn.transaction::<_, diesel::result::Error, _>(|| {
    //         // if already exists, we are happy! don't need to do anything :))
    //         if let Ok(_) = incoming_chunk::table.filter((incoming_chunk::from_friend.eq(chunk.from_friend), incoming_chunk::sequence_number.eq(chunk.sequence_number))).first(&conn) {
    //             return Ok(());
    //         }

    //         // check if there is already a message uid associated with this chunk sequence
    //         let q = incoming_chunk::table.filter((incoming_chunk::from_friend.eq(chunk.from_friend),incoming_chunk::chunks_start_sequence_number.eq(chunk.chunks_start_sequence_number)));
    //         let message_uid;
    //         match q.first(&conn) {
    //             Ok(ref_chunk) => {
    //                 message_uid = ref_chunk.message_uid;
    //                 // ok now we want to simply insert this chunk!
    //                 let insertable_chunk = IncomingChunk {
    //                     from_friend: chunk.from_friend,
    //                     sequence_number: chunk.sequence_number,
    //                     chunks_start_sequence_number: chunk.chunks_start_sequence_number,
    //                     message_uid: message_uid,
    //                     s: chunk.s,
    //                 };
    //                 diesel::insert_into(incoming_chunk::table).values(&insertable_chunk).execute(&conn)?;
    //             },
    //             Err(_) => {
    //                 // if there is no message uid associated with this chunk sequence, we need to create a new message
    //                 let new_msg = diesel::insert_into(message::table).values(message::s.eq("")).get_result(&conn)?;
    //                 message_uid = new_msg.uid;
    //                 let new_received = Received {
    //                     uid: message_uid,
    //                     from_friend: chunk.from_friend,
    //                     num_chunks: chunk.num_chunks,
    //                     received_at: Utc::now().timestamp(),
    //                     delivered: false,
    //                     delivered_at: None,
    //                     seen: false
    //                 };
    //                 diesel::insert_into(received::table).values(&new_received).execute(&conn)?;
    //                 let insertable_chunk = IncomingChunk {
    //                     from_friend: chunk.from_friend,
    //                     sequence_number: chunk.sequence_number,
    //                     chunks_start_sequence_number: chunk.chunks_start_sequence_number,
    //                     message_uid: message_uid,
    //                     s: chunk.s,
    //                 };
    //                 diesel::insert_into(incoming_chunk::table).values(&insertable_chunk).execute(&conn)?;
    //             }
    //         };
    //         // check if we have received all chunks!
    //         let q = incoming_chunk::table.filter((incoming_chunk::from_friend.eq(chunk.from_friend),incoming_chunk::chunks_start_sequence_number.eq(chunk.chunks_start_sequence_number)));
    //         let count = q.count().get_result(&conn)?;
    //         if count == chunk.num_chunks {
    //             // we have received all chunks!
    //             // now assemble the message, write it, and be happy!
    //             let all_chunks = q.order_by(incoming_chunk::sequence_number).load::<IncomingChunk>(&conn)?;
    //             let msg = all_chunks.iter().fold(String::new(), |mut acc, chunk| {
    //                 acc.push_str(&chunk.s);
    //                 acc
    //             });
    //             diesel::update(message::table.find(message_uid)).set((message::s.eq(msg), message::delivered.eq(true), message::delivered_at(Utc::now().timestamp()))).execute(&conn)?;
    //             // finally, delete the chunks
    //             diesel::delete(incoming_chunk::table.filter((incoming_chunk::from_friend.eq(chunk.from_friend), incoming_chunk::chunks_start_sequence_number.eq(chunk.chunks_start_sequence_number)))).execute(&conn)?;
    //         }
    //     };

    //     match r {
    //         Ok(_) => Ok(()),
    //         Err(e) => Err(DbError::Unknown(format!("receive_chunk: {}", e))),
    //     }
    // }

    

}