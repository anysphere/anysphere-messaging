// WHEN CHANGING THIS FILE, REMEMBER TO RUN MIGRATIONS.

table! {
    friends (uid) {
        uid -> Integer,
        unique_name -> Text,
        display_name -> Text,
        enabled -> Bool,
    }
}