#!/bin/bash

# DefendX Agent package generator
# Copyright (C) 2025, DefendX Inc.
#
# This program is a free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public
# License (version 2) as published by the FSF - Free Software
# Foundation.

set -ex
CURRENT_PATH="$( cd $(dirname $0) ; pwd -P )"
DEFENDX_PATH="$(cd $CURRENT_PATH/..; pwd -P)"
ARCHITECTURE="amd64"
SYSTEM="deb"
OUTDIR="${CURRENT_PATH}/output/"
BRANCH=""
VCPKG_KEY=""
VCPKG_BINARY_SOURCES=""
REVISION="0"
TARGET="agent"
JOBS="2"
DEBUG="no"
SRC="no"
BUILD_DOCKER="yes"
DOCKER_TAG="latest"
INSTALLATION_PATH="/"
CHECKSUM="no"
FUTURE="no"
IS_STAGE="no"
ENTRYPOINT="/home/build.sh"

trap ctrl_c INT

clean() {
    exit_code=$1
    find "${DOCKERFILE_PATH}" \( -name '*.sh' -o -name '*.tar.gz' -o -name 'defendx-*' \) ! -name 'docker_builder.sh' -exec rm -rf {} +
    exit ${exit_code}
}

ctrl_c() {
    clean 1
}

build_pkg() {
    CONTAINER_NAME="pkg_${SYSTEM}_${TARGET}_builder_${ARCHITECTURE}"
    DOCKERFILE_PATH="${CURRENT_PATH}/${SYSTEM}s/${ARCHITECTURE}/${TARGET}"

    cp ${CURRENT_PATH}/build.sh ${DOCKERFILE_PATH}
    cp ${CURRENT_PATH}/${SYSTEM}s/utils/* ${DOCKERFILE_PATH}

    if [[ ${BUILD_DOCKER} == "yes" ]]; then
        docker build -t ${CONTAINER_NAME}:${DOCKER_TAG} ${DOCKERFILE_PATH} || return 1
    fi

    docker run -t --rm -v ${OUTDIR}:/var/local/defendx:Z \
        -e SYSTEM="$SYSTEM" \
        -e BUILD_TARGET="${TARGET}" \
        -e ARCHITECTURE_TARGET="${ARCHITECTURE}" \
        -e INSTALLATION_PATH="${INSTALLATION_PATH}" \
        -e IS_STAGE="${IS_STAGE}" \
        -e DEFENDX_BRANCH="${BRANCH}" \
        -e DEFENDX_VERBOSE="${VERBOSE}" \
        -e VCPKG_KEY="${VCPKG_KEY}" \
        -e VCPKG_BINARY_SOURCES="${VCPKG_BINARY_SOURCES}" \
        ${CUSTOM_CODE_VOL} \
        -v ${DOCKERFILE_PATH}:/home:Z \
        ${CONTAINER_NAME}:${DOCKER_TAG} \
        ${ENTRYPOINT} \
        ${REVISION} ${JOBS} ${DEBUG} \
        ${CHECKSUM} ${FUTURE} ${SRC} || return 1

    echo "Package $(ls -Art ${OUTDIR} | tail -n 1) added to ${OUTDIR}."
    return 0
}

build() {
    build_pkg || return 1
    return 0
}

help() {
    set +x
    echo
    echo "Usage: $0 [OPTIONS]"
    echo
    echo "    -b, --branch <branch>      [Optional] Select Git branch."
    echo "    -a, --architecture <arch>  [Optional] Target architecture [amd64/i386/ppc64le/arm64/armhf]."
    echo "    -j, --jobs <number>        [Optional] Parallel jobs. Default: 2."
    echo "    -r, --revision <rev>       [Optional] Package revision. Default: 0."
    echo "    -s, --store <path>         [Optional] Output directory."
    echo "    -p, --path <path>          [Optional] Installation path. Default: /var/defendx."
    echo "    -d, --debug                [Optional] Enable debug mode."
    echo "    -c, --checksum             [Optional] Generate checksum."
    echo "    --dont-build-docker        [Optional] Use existing Docker image."
    echo "    --tag                      [Optional] Docker image tag."
    echo "    --sources <path>           [Optional] Use local source code."
    echo "    --is_stage                 [Optional] Use release name in package."
    echo "    --system                   [Optional] Select OS [rpm, deb]. Default: 'deb'."
    echo "    --src                      [Optional] Generate source package."
    echo "    --future                   [Optional] Build future package."
    echo "    --verbose                  [Optional] Print executed commands."
    echo "    -h, --help                 Show this help."
    echo
    exit $1
}

main() {
    while [ -n "$1" ]
    do
        case "$1" in
        "-b"|"--branch")
            BRANCH="$2"; shift 2;;
        "-h"|"--help")
            help 0;;
        "-a"|"--architecture")
            ARCHITECTURE="$2"; shift 2;;
        "-j"|"--jobs")
            JOBS="$2"; shift 2;;
        "-r"|"--revision")
            REVISION="$2"; shift 2;;
        "-p"|"--path")
            INSTALLATION_PATH="$2"; shift 2;;
        "-e"|"--entrypoint")
            ENTRYPOINT="$2"; shift 2;;
        "-d"|"--debug")
            DEBUG="yes"; shift 1;;
        "-c"|"--checksum")
            CHECKSUM="yes"; shift 1;;
        "--dont-build-docker")
            BUILD_DOCKER="no"; shift 1;;
        "--tag")
            DOCKER_TAG="$2"; shift 2;;
        "-s"|"--store")
            OUTDIR="$2"; shift 2;;
        "--sources")
            CUSTOM_CODE_VOL="-v $2:/defendx-local-src:Z"; shift 2;;
        "--future")
            FUTURE="yes"; shift 1;;
        "--is_stage")
            IS_STAGE="yes"; shift 1;;
        "--src")
            SRC="yes"; shift 1;;
        "--system")
            SYSTEM="$2"; shift 2;;
        "--verbose")
            VERBOSE="yes"; shift 1;;
        *)
            help 1;;
        esac
    done

    if [ -n "${VERBOSE}" ]; then
        set -x
    fi

    if [ -z "${CUSTOM_CODE_VOL}" ] && [ -z "${BRANCH}" ]; then
        CUSTOM_CODE_VOL="-v $DEFENDX_PATH:/defendx-local-src:Z"
    fi

    build && clean 0
    clean 1
}

main "$@"
