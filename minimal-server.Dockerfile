FROM debian:bullseye

RUN apt-get update && apt-get install -y libpq-dev

WORKDIR /app
COPY as_server /app

ENTRYPOINT ["/app/as_server"]
CMD ["-d", "34.73.92.64"]