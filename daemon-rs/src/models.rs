use diesel::prelude::*;

#[derive(Queryable)]
pub struct Friend {
    pub uid: i32,
    pub unique_name: String,
    pub display_name: String,
    pub enabled: bool,
}