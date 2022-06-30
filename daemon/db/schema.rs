// @generated automatically by Diesel CLI.

diesel::table! {
    complete_friend (friend_uid) {
        friend_uid -> Integer,
        public_id -> Text,
        invitation_public_key -> Binary,
        kx_public_key -> Binary,
        completed_at -> BigInt,
    }
}

diesel::table! {
    config (uid) {
        uid -> Integer,
        server_address -> Text,
        latency -> Integer,
        has_registered -> Bool,
        registration_uid -> Nullable<Integer>,
    }
}

diesel::table! {
    draft (uid) {
        uid -> Integer,
        to_friend -> Integer,
    }
}

diesel::table! {
    friend (uid) {
        uid -> Integer,
        unique_name -> Text,
        display_name -> Text,
        invitation_progress -> Integer,
        deleted -> Bool,
    }
}

diesel::table! {
    incoming_chunk (from_friend, sequence_number) {
        from_friend -> Integer,
        sequence_number -> Integer,
        chunks_start_sequence_number -> Integer,
        message_uid -> Integer,
        content -> Text,
    }
}

diesel::table! {
    incoming_invitation (public_id) {
        public_id -> Text,
        message -> Text,
        received_at -> BigInt,
    }
}

diesel::table! {
    message (uid) {
        uid -> Integer,
        content -> Text,
    }
}

diesel::table! {
    outgoing_async_invitation (friend_uid) {
        friend_uid -> Integer,
        public_id -> Text,
        invitation_public_key -> Binary,
        kx_public_key -> Binary,
        message -> Text,
        sent_at -> BigInt,
    }
}

diesel::table! {
    outgoing_chunk (to_friend, sequence_number) {
        to_friend -> Integer,
        sequence_number -> Integer,
        chunks_start_sequence_number -> Integer,
        message_uid -> Nullable<Integer>,
        content -> Text,
        system -> Bool,
        system_message -> Integer,
    }
}

diesel::table! {
    outgoing_sync_invitation (friend_uid) {
        friend_uid -> Integer,
        story -> Text,
        kx_public_key -> Binary,
        sent_at -> BigInt,
    }
}

diesel::table! {
    received (uid) {
        uid -> Integer,
        from_friend -> Integer,
        num_chunks -> Integer,
        received_at -> BigInt,
        delivered -> Bool,
        delivered_at -> Nullable<BigInt>,
        seen -> Bool,
    }
}

diesel::table! {
    registration (uid) {
        uid -> Integer,
        invitation_public_key -> Binary,
        invitation_private_key -> Binary,
        kx_public_key -> Binary,
        kx_private_key -> Binary,
        allocation -> Integer,
        pir_secret_key -> Binary,
        pir_galois_key -> Binary,
        authentication_token -> Text,
        public_id -> Text,
    }
}

diesel::table! {
    sent (uid) {
        uid -> Integer,
        to_friend -> Integer,
        num_chunks -> Integer,
        sent_at -> BigInt,
        delivered -> Bool,
        delivered_at -> Nullable<BigInt>,
    }
}

diesel::table! {
    transmission (friend_uid) {
        friend_uid -> Integer,
        read_index -> Integer,
        read_key -> Binary,
        write_key -> Binary,
        ack_index -> Integer,
        sent_acked_seqnum -> Integer,
        received_seqnum -> Integer,
    }
}

diesel::joinable!(complete_friend -> friend (friend_uid));
diesel::joinable!(config -> registration (registration_uid));
diesel::joinable!(draft -> friend (to_friend));
diesel::joinable!(draft -> message (uid));
diesel::joinable!(incoming_chunk -> friend (from_friend));
diesel::joinable!(incoming_chunk -> received (message_uid));
diesel::joinable!(outgoing_async_invitation -> friend (friend_uid));
diesel::joinable!(outgoing_chunk -> friend (to_friend));
diesel::joinable!(outgoing_chunk -> sent (message_uid));
diesel::joinable!(outgoing_sync_invitation -> friend (friend_uid));
diesel::joinable!(received -> friend (from_friend));
diesel::joinable!(received -> message (uid));
diesel::joinable!(sent -> friend (to_friend));
diesel::joinable!(sent -> message (uid));
diesel::joinable!(transmission -> friend (friend_uid));

diesel::allow_tables_to_appear_in_same_query!(
  complete_friend,
  config,
  draft,
  friend,
  incoming_chunk,
  incoming_invitation,
  message,
  outgoing_async_invitation,
  outgoing_chunk,
  outgoing_sync_invitation,
  received,
  registration,
  sent,
  transmission,
);
