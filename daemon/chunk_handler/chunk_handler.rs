
#[cxx::bridge(namespace = "chunk_handler")]
pub mod chunk_handler {
  struct MsgProto {
    other_recipients: Vec<String>,
    msg: String,
  }

  unsafe extern "C++" {
    include!("daemon/chunk_handler/chunk_handler.hpp");

    fn chunks_to_msg(chunks: Vec<u8>) -> Result<MsgProto>;
  }
}

