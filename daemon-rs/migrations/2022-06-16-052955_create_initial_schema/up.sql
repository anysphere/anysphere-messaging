CREATE TABLE config (
    server_address text NOT NULL,
    latency integer NOT NULL,
    db_rows integer NOT NULL,
    has_registered boolean NOT NULL,
    public_key blob,
    private_key blob,
    allocation integer,
    pir_secret_key blob,
    pir_galois_key blob,
    authentication_token text
);

CREATE TABLE friend (
    uid integer PRIMARY KEY NOT NULL,
    unique_name text NOT NULL,
    display_name text NOT NULL,
    enabled boolean NOT NULL,
    deleted boolean NOT NULL
);

CREATE TABLE address (
    uid integer PRIMARY KEY NOT NULL,
    read_index integer NOT NULL,
    ack_index integer NOT NULL,
    read_key blob NOT NULL,
    write_key blob NOT NULL,
    FOREIGN KEY(uid) REFERENCES friend(uid)
);

CREATE TABLE status (
    uid integer PRIMARY KEY NOT NULL,
    sent_acked_seqnum integer NOT NULL, -- the maximum ACK from this friend
    received_seqnum integer NOT NULL, -- we have received all messages up and including this seqnum
    FOREIGN KEY(uid) REFERENCES friend(uid)
)

-- message includes ALL real messages
CREATE TABLE message (
    uid integer PRIMARY KEY NOT NULL,
    message text NOT NULL
);

CREATE TABLE draft (
    uid integer PRIMARY KEY NOT NULL,
    to_friend integer NOT NULL,
    FOREIGN KEY(uid) REFERENCES message(uid),
    FOREIGN KEY(to_friend) REFERENCES friend(uid)
);

-- sent includes messages that have only partially been sent, and still have chunks (delivered=false)
CREATE TABLE sent (
    uid integer PRIMARY KEY NOT NULL,
    to_friend integer NOT NULL,
    num_chunks integer NOT NULL,
    sent_at timestamp NOT NULL, -- time when the user pressed 'Send'
    delivered boolean NOT NULL, -- true when the entire message has been delivered and acked
    delivered_at timestamp, -- time when the message was delivered
    FOREIGN KEY(uid) REFERENCES message(uid),
    FOREIGN KEY(to_friend) REFERENCES friend(uid)
);

-- received includes messages that have only partially been received, and still have chunks (delivered=false)
CREATE TABLE received (
    uid integer PRIMARY KEY NOT NULL,
    from_friend integer NOT NULL,
    num_chunks integer NOT NULL,
    received_at timestamp NOT NULL, -- timestamp when the first chunk was received
    delivered boolean NOT NULL, -- true when the entire message has been delivered
    delivered_at timestamp, -- timestamp when the last chunk was delivered
    seen boolean NOT NULL,
    FOREIGN KEY(uid) REFERENCES message(uid),
    FOREIGN KEY(from_friend) REFERENCES friend(uid)
);

CREATE TABLE outgoing_chunk (
    to_friend integer NOT NULL,
    sequence_number integer NOT NULL,
    chunk_index integer NOT NULL,
    message_uid integer NOT NULL,
    message text NOT NULL,
    PRIMARY KEY (to_friend, sequence_number),
    FOREIGN KEY(message_uid) REFERENCES sent(uid),
    FOREIGN KEY(to_friend) REFERENCES friend(uid)
);

CREATE TABLE incoming_chunk (
    from_friend integer NOT NULL,
    sequence_number integer NOT NULL,
    chunk_index integer NOT NULL,
    message_uid integer NOT NULL,
    message text NOT NULL,
    PRIMARY KEY (from_friend, sequence_number),
    FOREIGN KEY(message_uid) REFERENCES received(uid),
    FOREIGN KEY(from_friend) REFERENCES friend(uid)
);