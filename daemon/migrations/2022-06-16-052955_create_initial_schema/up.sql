-- exactly 1 element always!
-- DUPLICATION: MAKE SURE TO HAVE THE CONSTANTS BE THE SAME AS constants.hpp
-- When you update the constants.hpp, you need to run a migration here.
CREATE TABLE config (
    uid integer PRIMARY KEY NOT NULL,
    server_address text NOT NULL,
    latency integer NOT NULL,
  -- invariant: has_registered can never become false after being true.
    has_registered boolean NOT NULL,
    registration_uid integer,
    FOREIGN KEY(registration_uid) REFERENCES registration(uid)
);

-- 0-1 elements always!
CREATE TABLE registration (
    uid integer PRIMARY KEY NOT NULL,
    public_key blob NOT NULL,
    private_key blob NOT NULL,
    allocation integer NOT NULL,
    pir_secret_key blob NOT NULL,
    pir_galois_key blob NOT NULL,
    authentication_token text NOT NULL
);

-- never delete a friend! instead, set `deleted` to true, or else we will lose history!
-- (if you actually do delete, you need to also delete from the message tables, or else 
-- the foreign key constraints will fail)
CREATE TABLE friend (
    uid integer PRIMARY KEY NOT NULL,
    unique_name text UNIQUE NOT NULL,
    display_name text NOT NULL,
    enabled boolean NOT NULL,
    deleted boolean NOT NULL
);

CREATE TABLE address (
    uid integer PRIMARY KEY NOT NULL,
    read_index integer NOT NULL,
  -- ack_index is the index into the acking data for this friend
  -- this NEEDS to be unique for every friend!!
  -- This needs to be between 0 <= ack_index < MAX_FRIENDS
    ack_index integer NOT NULL,
    read_key blob NOT NULL,
    write_key blob NOT NULL,
    FOREIGN KEY(uid) REFERENCES friend(uid)
);

CREATE TABLE status (
    uid integer PRIMARY KEY NOT NULL,
  -- sent_acked_seqnum is the latest sequence number that was ACKed by the friend
  -- any message with seqnum > sent_acked_seqnum MUST be retried.
    sent_acked_seqnum integer NOT NULL, 
  -- received_seqnum is the value that should be ACKed. we acknowledge that we
  -- have received all sequence numbers up to and including this value.
    received_seqnum integer NOT NULL,
    FOREIGN KEY(uid) REFERENCES friend(uid)
);

-- message includes ALL real messages
CREATE TABLE message (
    uid integer PRIMARY KEY NOT NULL,
    content text NOT NULL -- the payload!
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
    delivered_at timestamp UNIQUE, -- time when the message was delivered. 
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
    delivered_at timestamp UNIQUE, -- timestamp when the last chunk was delivered. unique because we want to use it as a monotonically increasing index in the order that messages are delivered. since we use microseconds collisions are unlikely, and if there is a collision we can just retry the entire transaction.
    seen boolean NOT NULL,
    FOREIGN KEY(uid) REFERENCES message(uid),
    FOREIGN KEY(from_friend) REFERENCES friend(uid)
);

CREATE TABLE outgoing_chunk (
    to_friend integer NOT NULL,
    sequence_number integer NOT NULL,
    chunks_start_sequence_number integer NOT NULL,
    message_uid integer NOT NULL,
    content text NOT NULL,
    PRIMARY KEY (to_friend, sequence_number),
    FOREIGN KEY(message_uid) REFERENCES sent(uid),
    FOREIGN KEY(to_friend) REFERENCES friend(uid)
);

CREATE TABLE incoming_chunk (
    from_friend integer NOT NULL,
    sequence_number integer NOT NULL,
    chunks_start_sequence_number integer NOT NULL,
    message_uid integer NOT NULL,
    content text NOT NULL,
    PRIMARY KEY (from_friend, sequence_number),
    FOREIGN KEY(message_uid) REFERENCES received(uid),
    FOREIGN KEY(from_friend) REFERENCES friend(uid)
);






------------------END OF SCHEMA------------------
------------------START OF DATA------------------

-- DUPLICATION: MAKE SURE TO HAVE THE CONSTANTS BE THE SAME AS constants.hpp
INSERT INTO config (uid, server_address, latency, has_registered) VALUES (1, "server1.anysphere.co:443", 30, false);