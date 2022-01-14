.PHONY: server minimal-server push-minimal clean push distribute build-client

server: 
	DOCKER_BUILDKIT=1 docker build -f server.Dockerfile -t server .

minimal-server: 
	rm -f as_server
	cp /workspace/bazel_output_base/execroot/__main__/bazel-out/k8-fastbuild/bin/server/src/as_server as_server
	DOCKER_BUILDKIT=1 docker build -f minimal-server.Dockerfile -t minimal-server .
	rm -f as_server

push: server
	docker tag server us-east1-docker.pkg.dev/veil-messenger/server/server
	docker push us-east1-docker.pkg.dev/veil-messenger/server/server

push-minimal: minimal-server
	docker tag minimal-server us-east1-docker.pkg.dev/veil-messenger/server/server
	docker push us-east1-docker.pkg.dev/veil-messenger/server/server

build-client:
	bazel build //client/... 

# TODO: add release arguments here to speed up things
distribute: build-client
	pushd client/gui2
	npm run update
	npm run packall
	popd
	cp -r client/gui2/release release
	echo "Client successfully built!"

clean:
	docker rmi -f server
	docker rmi -f minimal-server