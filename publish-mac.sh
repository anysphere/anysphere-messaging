#!/usr/bin/env bash

sed -i '' "s/version.*/version\": \"${ASPHR_VERSION}\",/g" client/gui/release/app/package.json
pushd client/gui || exit
npm install
PUBLISH_ASPHR=true npm run package-mac
popd || exit

# update version!
sed -i '' "s/ASPHR_VERSION =.*/ASPHR_VERSION = \"${ASPHR_VERSION}\";/g" website/landing/src/pages/download.tsx
echo "Updated version. Please deploy the landing page, and publish the release, when ready."
