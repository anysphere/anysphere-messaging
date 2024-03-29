CREATE SCHEMA IF NOT EXISTS server;

CREATE TABLE IF NOT EXISTS accounts (
  id INT GENERATED ALWAYS AS IDENTITY,
  public_key BYTEA NOT NULL,
  authentication_token VARCHAR(255) NOT NULL,
  pir_index INT NOT NULL
);