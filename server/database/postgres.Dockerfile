FROM postgres
ENV POSTGRES_DB postgres
ENV POSTGRES_USER postgres
ENV POSTGRES_PASSWORD 3b125115d91aeef4724a0c81bed8fce6782f8360b3b8c36611
COPY schema.sql /docker-entrypoint-initdb.d/