// @generated automatically by Diesel CLI.

diesel::table! {
    address (uid) {
        uid -> Integer,
        read_index -> Integer,
        ack_index -> Integer,
        read_key -> Binary,
        write_key -> Binary,
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
        enabled -> Bool,
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
    message (uid) {
        uid -> Integer,
        content -> Text,
    }
}

diesel::table! {
    outgoing_chunk (to_friend, sequence_number) {
        to_friend -> Integer,
        sequence_number -> Integer,
        chunks_start_sequence_number -> Integer,
        message_uid -> Integer,
        content -> Text,
    }
}

diesel::table! {
    received (uid) {
        uid -> Integer,
        from_friend -> Integer,
        num_chunks -> Integer,
        received_at -> TimestamptzSqlite,
        delivered -> Bool,
        delivered_at -> Nullable<TimestamptzSqlite>,
        seen -> Bool,
    }
}

diesel::table! {
    registration (uid) {
        uid -> Integer,
        public_key -> Binary,
        private_key -> Binary,
        allocation -> Integer,
        pir_secret_key -> Binary,
        pir_galois_key -> Binary,
        authentication_token -> Text,
    }
}

diesel::table! {
    sent (uid) {
        uid -> Integer,
        to_friend -> Integer,
        num_chunks -> Integer,
        sent_at -> TimestamptzSqlite,
        delivered -> Bool,
        delivered_at -> Nullable<TimestamptzSqlite>,
    }
}

diesel::table! {
    status (uid) {
        uid -> Integer,
        sent_acked_seqnum -> Integer,
        received_seqnum -> Integer,
    }
}

diesel::joinable!(address -> friend (uid));
diesel::joinable!(config -> registration (registration_uid));
diesel::joinable!(draft -> friend (to_friend));
diesel::joinable!(draft -> message (uid));
diesel::joinable!(incoming_chunk -> friend (from_friend));
diesel::joinable!(incoming_chunk -> received (message_uid));
diesel::joinable!(outgoing_chunk -> friend (to_friend));
diesel::joinable!(outgoing_chunk -> sent (message_uid));
diesel::joinable!(received -> friend (from_friend));
diesel::joinable!(received -> message (uid));
diesel::joinable!(sent -> friend (to_friend));
diesel::joinable!(sent -> message (uid));
diesel::joinable!(status -> friend (uid));

diesel::allow_tables_to_appear_in_same_query!(
    address,
    config,
    draft,
    friend,
    incoming_chunk,
    message,
    outgoing_chunk,
    received,
    registration,
    sent,
    status,
);
