FROM debian:bullseye

RUN apt-get update && apt-get install -y libpq-dev && apt-get install -y postgresql-client

WORKDIR /app
COPY as_server /app
COPY init_db_and_run_server.sh /app
COPY server/database/schema.sql /app

ENTRYPOINT ["/app/init_db_and_run_server.sh"]
CMD ["db_host_name", "db_password"]