#!/usr/bin/env bash

if [[ -z ${ASPHR_VERSION} ]]; then
	echo "ASPHR_VERSION is empty. Please set it to the version of asphr you want to publish."
	exit 1
fi

sd "version.*" "version\": \"${ASPHR_VERSION}\"," client/gui/release/app/package.json
pushd client/gui || exit
npm install
PUBLISH_ASPHR=true npm run package-linux
popd || exit

# update version!
sd "LINUX_ASPHR_VERSION =.*" "LINUX_ASPHR_VERSION = \"${ASPHR_VERSION}\";" website/landing/src/pages/download.tsx
sd "LINUX_ASPHR_VERSION =.*" "LINUX_ASPHR_VERSION = \"${ASPHR_VERSION}\";" website/landing-experiment/src/pages/download.tsx
echo "Updated version. Please deploy the landing page, and publish the release, when ready."
