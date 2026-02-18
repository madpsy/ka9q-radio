#!/bin/bash

VERSION=0.8.1
IMAGE=madpsy/ka9q-radio

# Parse command line arguments
SUFFIX=""
while [[ $# -gt 0 ]]; do
  case $1 in
    --no-avx2)
      SUFFIX="-no-avx2"
      shift
      ;;
    *)
      echo "Unknown option: $1"
      echo "Usage: $0 [--no-avx2]"
      exit 1
      ;;
  esac
done

# Build image with version tag
docker build -t $IMAGE:$VERSION$SUFFIX -f docker/Dockerfile .

# Tag version as latest
docker tag $IMAGE:$VERSION$SUFFIX $IMAGE:latest$SUFFIX

# Push both tags
docker push $IMAGE:$VERSION$SUFFIX
docker push $IMAGE:latest$SUFFIX
