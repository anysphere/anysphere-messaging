#!/usr/bin/env bash

if [[ -z "${ASPHR_VERSION}" ]]; then
    echo "ASPHR_VERSION is empty. Please set it to the version of asphr you want to publish."
    exit 1
fi

sed -i '' "s/version.*/version\": \"${ASPHR_VERSION}\",/g" client/gui/release/app/package.json
pushd client/gui || exit
npm install
PUBLISH_ASPHR=true npm run package-mac
popd || exit

# update version!
sed -i '' "s/ASPHR_VERSION =.*/ASPHR_VERSION = \"${ASPHR_VERSION}\";/g" website/landing/src/pages/download.tsx
echo "Updated version. Please deploy the landing page, and publish the release, when ready."
