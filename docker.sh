#!/bin/bash

VERSION=0.8.1
IMAGE=madpsy/ka9q-radio

# Parse command line arguments
SUFFIX=""
CUSTOM_VERSION=""
TAG_LATEST=true
NO_AVX2=false

while [[ $# -gt 0 ]]; do
  case $1 in
    --no-avx2)
      SUFFIX="-no-avx2"
      NO_AVX2=true
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
      echo "  --no-avx2         Build without AVX2 support (amd64 only)"
      echo "  --version VERSION Tag with specific version (does not tag as latest)"
      exit 1
      ;;
  esac
done

# Use custom version if provided, otherwise use default
if [ -n "$CUSTOM_VERSION" ]; then
  VERSION="$CUSTOM_VERSION"
fi

# Ensure a buildx builder with multi-platform support exists
BUILDER_NAME="ka9q-multiplatform"
if ! docker buildx inspect "$BUILDER_NAME" >/dev/null 2>&1; then
  echo "Creating buildx builder: $BUILDER_NAME"
  docker buildx create --name "$BUILDER_NAME" --driver docker-container --bootstrap
fi
docker buildx use "$BUILDER_NAME"

# --no-avx2 is an amd64-only concept; arm64 has no AVX2 so only build amd64 for that variant.
# Default (no suffix) builds a true multi-arch manifest covering both amd64 and arm64.
if [ "$NO_AVX2" = true ]; then
  PLATFORMS="linux/amd64"
else
  PLATFORMS="linux/amd64,linux/arm64"
fi

# Collect tags to push in a single buildx invocation
TAGS=()
TAGS+=("--tag" "$IMAGE:$VERSION$SUFFIX")
if [ "$TAG_LATEST" = true ]; then
  TAGS+=("--tag" "$IMAGE:latest$SUFFIX")
fi

echo "Building for platforms: $PLATFORMS"
echo "Tags: ${TAGS[*]}"

docker buildx build \
  --platform "$PLATFORMS" \
  "${TAGS[@]}" \
  --file docker/Dockerfile \
  --push \
  .
