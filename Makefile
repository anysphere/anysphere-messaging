.PHONY: server minimal-server push-minimal clean push distribute build-client

server: 
	DOCKER_BUILDKIT=1 docker build -f server.Dockerfile -t server .

minimal-server: 
	bazel build //server/src:as_server -c opt
	rm -f as_server
	cp /workspace/bazel_output_base/execroot/__main__/bazel-out/k8-opt/bin/server/src/as_server as_server
	DOCKER_BUILDKIT=1 docker build -f minimal-server.Dockerfile -t minimal-server .
	rm -f as_server

push: server
	docker tag server us-east1-docker.pkg.dev/veil-messenger/server/server
	docker push us-east1-docker.pkg.dev/veil-messenger/server/server

push-minimal-gcloud: minimal-server
	docker tag minimal-server us-east1-docker.pkg.dev/veil-messenger/server/server
	docker push us-east1-docker.pkg.dev/veil-messenger/server/server

push-minimal-aws: minimal-server
	docker tag minimal-server 946207870883.dkr.ecr.us-east-1.amazonaws.com/asphr-server
	docker push 946207870883.dkr.ecr.us-east-1.amazonaws.com/asphr-server

package-mac:
	pushd client/gui && npm install && npm run package-mac && popd
	cp -r client/gui/release/build/*.pkg release
	echo "Client successfully built for mac! Look in the release folder."

clean:
	docker rmi -f server
	docker rmi -f minimal-server