.PHONY: image run clean all

all:
	@echo "Please run 'make run' to run postgres. 'make clean' to stop and clean"

image: clean
	docker build -f postgres.Dockerfile -t as-postgres .

run: image
	docker run -p 5432:5432 --name as_db -d as-postgres

run-interactive: image
	docker run -p 5432:5432 --name as_db -it as-postgres

clean:
	docker stop as_db || true
	docker rm -f as_db
	docker rmi -f as-postgres