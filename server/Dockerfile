FROM debian:buster

RUN sudo apt-get update && sudo apt-get install libpq-dev

COPY .. /app
WORKDIR /app

RUN bazel build //server/src:as_server

ENTRYPOINT ["/bazel-bin/server/src/as_server"]