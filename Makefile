.PHONY: server minimal-server push push-minimal package-mac publish-mac-alpha update-gui-proto clean

server: 
	echo "WARNING: THE server.Dockerfile IS OUT OF DATE. PLEASE UPDATE IT, TAKING INSPIRATION FROM minimal-server.Dockerfile."
	DOCKER_BUILDKIT=1 docker build -f server.Dockerfile -t server .

minimal-server: 
	bazel build //server/src:as_server -c opt
	rm -f as_server
	# do echo "$(bazel info output_path)/k8-opt/bin/server/src/as_server"; makefile doesn't support $() syntax
	cp /workspace/root_bazel_output_base/execroot/__main__/bazel-out/k8-opt/bin/server/src/as_server as_server
	DOCKER_BUILDKIT=1 docker build -f minimal-server.Dockerfile -t minimal-server .
	rm -f as_server

push: server
	docker tag server 946207870883.dkr.ecr.us-east-1.amazonaws.com/asphr-server
	docker push 946207870883.dkr.ecr.us-east-1.amazonaws.com/asphr-server

# ALWAYS ALWAYS RUN IN GITPOD. NEVER WITHOUT GITPOD
push-minimal: minimal-server
	docker tag minimal-server 946207870883.dkr.ecr.us-east-1.amazonaws.com/asphr-server
	docker push 946207870883.dkr.ecr.us-east-1.amazonaws.com/asphr-server
	echo "Now update infra/aws/variables.tf to include the new sha256 in the asphr_server_image_tag variable!"
	echo "Now do cd infra/aws && terraform apply"

package-mac-arm64:
	pushd client/gui && npm install && npm run package-mac-arm64 && popd
	./move-pkg.sh
	echo "Client successfully built for mac-arm64! Look in the release folder."

package-mac-x86_64:
	pushd client/gui && npm install && npm run package-mac-x86_64 && popd
	./move-pkg.sh x86_64
	echo "Client successfully built for mac-x86_64! Look in the release folder."

# we generate a random release ID so that only people we send the link to can download
publish-alpha: package-mac-arm64 package-mac-x86_64
	./publish.sh arm64 x86_64
	echo "Client successfully published to s3! Download from URL above."

# whenver daemon.proto is changed, run this on the server and push the changed files!
update-gui-proto:
	cd client/gui && npm run update

clean:
	docker rmi -f server
	docker rmi -f minimal-server