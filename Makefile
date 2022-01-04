.PHONY: server clean push

server: 
	DOCKER_BUILDKIT=1 docker build -f server.Dockerfile -t server .

push: server
	docker tag server us-east1-docker.pkg.dev/veil-messenger/server/server
	docker push us-east1-docker.pkg.dev/veil-messenger/server/server

clean:
	docker rmi -f server