#!/bin/bash

VERSION=0.8.1
IMAGE=madpsy/ka9q-radio

# Parse command line arguments
SUFFIX=""
CUSTOM_VERSION=""
TAG_LATEST=true

while [[ $# -gt 0 ]]; do
  case $1 in
    --no-avx2)
      SUFFIX="-no-avx2"
      shift
      ;;
    --version)
      CUSTOM_VERSION="$2"
      TAG_LATEST=false
      shift 2
      ;;
    *)
      echo "Unknown option: $1"
      echo "Usage: $0 [--no-avx2] [--version VERSION]"
      echo "  --no-avx2         Build without AVX2 support"
      echo "  --version VERSION Tag with specific version (does not tag as latest)"
      exit 1
      ;;
  esac
done

# Use custom version if provided, otherwise use default
if [ -n "$CUSTOM_VERSION" ]; then
  VERSION="$CUSTOM_VERSION"
fi

# Build image with version tag
docker build -t $IMAGE:$VERSION$SUFFIX -f docker/Dockerfile .

# Tag version as latest only if not using custom version
if [ "$TAG_LATEST" = true ]; then
  docker tag $IMAGE:$VERSION$SUFFIX $IMAGE:latest$SUFFIX
fi

# Push version tag
docker push $IMAGE:$VERSION$SUFFIX

# Push latest tag only if we tagged it
if [ "$TAG_LATEST" = true ]; then
  docker push $IMAGE:latest$SUFFIX
fi
