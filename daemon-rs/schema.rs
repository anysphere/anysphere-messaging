table! {
    address (uid) {
        uid -> Integer,
        read_index -> Integer,
        ack_index -> Integer,
        read_key -> Binary,
        write_key -> Binary,
    }
}

table! {
    config (uid) {
        uid -> Integer,
        server_address -> Nullable<Text>,
        server_address_default -> Bool,
        latency -> Nullable<Integer>,
        latency_default -> Bool,
        has_registered -> Bool,
        registration_uid -> Nullable<Integer>,
    }
}

table! {
    draft (uid) {
        uid -> Integer,
        to_friend -> Integer,
    }
}

table! {
    friend (uid) {
        uid -> Integer,
        unique_name -> Text,
        display_name -> Text,
        enabled -> Bool,
        deleted -> Bool,
    }
}

table! {
    incoming_chunk (from_friend, sequence_number) {
        from_friend -> Integer,
        sequence_number -> Integer,
        chunk_index -> Integer,
        message_uid -> Integer,
        message -> Text,
    }
}

table! {
    message (uid) {
        uid -> Integer,
        s -> Text,
    }
}

table! {
    outgoing_chunk (to_friend, sequence_number) {
        to_friend -> Integer,
        sequence_number -> Integer,
        chunk_index -> Integer,
        message_uid -> Integer,
        message -> Text,
    }
}

table! {
    received (uid) {
        uid -> Integer,
        from_friend -> Integer,
        num_chunks -> Integer,
        received_at -> Timestamp,
        delivered -> Bool,
        delivered_at -> Nullable<Timestamp>,
        seen -> Bool,
    }
}

table! {
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

table! {
    sent (uid) {
        uid -> Integer,
        to_friend -> Integer,
        num_chunks -> Integer,
        sent_at -> Timestamp,
        delivered -> Bool,
        delivered_at -> Nullable<Timestamp>,
    }
}

table! {
    status (uid) {
        uid -> Integer,
        sent_acked_seqnum -> Integer,
        received_seqnum -> Integer,
    }
}

joinable!(address -> friend (uid));
joinable!(config -> registration (registration_uid));
joinable!(draft -> friend (to_friend));
joinable!(draft -> message (uid));
joinable!(incoming_chunk -> friend (from_friend));
joinable!(incoming_chunk -> received (message_uid));
joinable!(outgoing_chunk -> friend (to_friend));
joinable!(outgoing_chunk -> sent (message_uid));
joinable!(received -> friend (from_friend));
joinable!(received -> message (uid));
joinable!(sent -> friend (to_friend));
joinable!(sent -> message (uid));
joinable!(status -> friend (uid));

allow_tables_to_appear_in_same_query!(
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
