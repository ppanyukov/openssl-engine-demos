#!/usr/bin/env bash

set -e

# Build script for this example.
# Supoprts Linux, and OS X.
# Set USE_DOCKER env var to build in docker container.

ROOT_DIR=$(cd $(dirname ${BASH_SOURCE}) && pwd)
cd ${ROOT_DIR}

function err {
    echo $@ 1>&2
}

if test "${USE_DOCKER}"
then
    IMAGE_NAME="openssl-cms-engine-go-$(id -u)"
    err "Will build and run in a docker image ${IMAGE_NAME}"
    docker build --tag ${IMAGE_NAME} .
    docker run -it --rm -v ${PWD}:${PWD} -w ${PWD} ${IMAGE_NAME} ./build.sh
    exit $?
fi

# Standard gcc flags to use
GCC_FLAGS="-fPIC"

if test "$(uname)" == "Darwin"
then
    err "On OS X openssl needs to be install using brew."
    err "e.g. brew install openssl"
    err "This usually gets installed into /usr/local/opt/openssl"
    err "If something does not work check that location"
    GCC_FLAGS="${GCC_FLAGS} -I /usr/local/opt/openssl/include"
    LD_FLAGS="-L /usr/local/opt/openssl/lib"
    OUT_FILE="-o engine.dylib"
elif test "$(uname)" == "Linux"
then
    err "On Linux need to install the following packages (Centos):"
    err "  yum -y install gcc"
    err "  yum -y install openssl-devel"
    GCC_FLAGS="${GCC_FLAGS} -I /usr/local/opt/openssl/include"
    LD_FLAGS=""
    OUT_FILE="-o engine.so"
else
    err "Unsupported platform: $(uname)"
    exit 1
fi

# Note the absence of direct calls to gcc, clang, and ld.
# All we do is tell go to do all the work.
#
# Additional compiler and linker params are passed via
# CGO_CFLAGS and CGO_LDFLAGS.
err "Build engine"
set -x
CGO_CFLAGS="${GCC_FLAGS}" CGO_LDFLAGS="${LD_FLAGS}" go build -buildmode=c-shared ${OUT_FILE}
{ set +x; } 2>/dev/null
err "DONE Build engine"
