table! {
    draft (uid) {
        uid -> Integer,
        to_friend -> Text,
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
    message (uid) {
        uid -> Integer,
        message -> Text,
    }
}

table! {
    sent (uid) {
        uid -> Integer,
        to_friend -> Text,
        sent_at -> Timestamp,
        delivered -> Bool,
    }
}

joinable!(draft -> message (uid));
joinable!(sent -> message (uid));

allow_tables_to_appear_in_same_query!(
    draft,
    friend,
    message,
    sent,
);
