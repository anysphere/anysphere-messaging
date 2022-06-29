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

-- IMPORTANT: friend request key always allocated before kx key.
-- 0-1 elements always!
CREATE TABLE registration (
    uid integer PRIMARY KEY NOT NULL,
    friend_request_public_key blob NOT NULL,
    friend_request_private_key blob NOT NULL,
    kx_public_key blob NOT NULL,
    kx_private_key blob NOT NULL,
    allocation integer NOT NULL,
    pir_secret_key blob NOT NULL,
    pir_galois_key blob NOT NULL,
    authentication_token text NOT NULL,
    public_id text NOT NULL -- redundant, but as this is used so prevalently, we'll keep it.
);

-- invitation does not create a friend, because we do not want to pollute friend space with non-friends
CREATE TABLE incoming_invitation (
    public_id text PRIMARY KEY NOT NULL,
    message text NOT NULL,
    received_at timestamp NOT NULL
);

-- never delete a friend! instead, set `deleted` to true, or else we will lose history!
-- (if you actually do delete, you need to also delete from the message tables, or else 
-- the foreign key constraints will fail)
--
-- a friend should exist in this table iff it should be possible to send and receive from the friend
-- this is true for all the outgoing invitations and complete friends, but not for incoming invitations.
-- hence, incoming invitations are their own table without a foreign key to friend, whereas all other
-- invitations have a foreign key to friend.
CREATE TABLE friend (
    uid integer PRIMARY KEY NOT NULL,
    unique_name text UNIQUE NOT NULL,
    display_name text NOT NULL,
    invitation_progress integer NOT NULL, -- enum defined in db.rs. outgoingasync, outgoingsync, complete.
    deleted boolean NOT NULL
);

CREATE TABLE outgoing_sync_invitation (
    friend_uid integer PRIMARY KEY NOT NULL,
    story text NOT NULL,
    kx_public_key blob NOT NULL,
    sent_at timestamp NOT NULL,
    FOREIGN KEY(friend_uid) REFERENCES friend(uid)
);

CREATE TABLE outgoing_async_invitation (
    friend_uid integer PRIMARY KEY NOT NULL,
    public_id text NOT NULL,
    friend_request_public_key blob NOT NULL,
    kx_public_key blob NOT NULL,
    message text NOT NULL,
    sent_at timestamp NOT NULL,
    FOREIGN KEY(friend_uid) REFERENCES friend(uid)
);

CREATE TABLE complete_friend (
    friend_uid integer PRIMARY KEY NOT NULL,
    public_id text NOT NULL,
    friend_request_public_key blob NOT NULL,
    kx_public_key blob NOT NULL,
    completed_at timestamp NOT NULL,
    FOREIGN KEY(friend_uid) REFERENCES friend(uid)
);

-- transmission table iff friend is !deleted
CREATE TABLE transmission (
    friend_uid integer PRIMARY KEY NOT NULL,
    read_index integer NOT NULL,
    read_key blob NOT NULL,
    write_key blob NOT NULL,
  -- ack_index is the index into the acking data for this friend
  -- this NEEDS to be unique for every friend!!
  -- This needs to be between 0 <= ack_index < MAX_FRIENDS
    ack_index integer NOT NULL, 
  -- sent_acked_seqnum is the latest sequence number that was ACKed by the friend
  -- any message with seqnum > sent_acked_seqnum MUST be retried.
    sent_acked_seqnum integer NOT NULL, 
  -- received_seqnum is the value that should be ACKed. we guarantee that we
  -- have received all sequence numbers up to and including this value.
    received_seqnum integer NOT NULL,
    FOREIGN KEY(friend_uid) REFERENCES friend(uid)
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
    message_uid integer, -- null iff system message
    content text NOT NULL,
    system boolean NOT NULL,
    system_message integer NOT NULL, -- corresponds to the enum value in the protobuf
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
INSERT INTO config (uid, server_address, latency, has_registered) VALUES (1, "server1.anysphere.co:443", 60, false);