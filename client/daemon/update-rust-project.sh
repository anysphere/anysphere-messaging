#!/usr/bin/env bash

if [[ $(sd --version || true) != *"sd"* ]]; then
	echo "Installing sd..."
	cargo install sd
fi

sd rust_static_library rust_library "${ANYSPHEREROOT}"/client/daemon/BUILD
bazelisk run @rules_rust//tools/rust_analyzer:gen_rust_project
sd rust_library rust_static_library "${ANYSPHEREROOT}"/client/daemon/BUILD
