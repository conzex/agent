#!/bin/bash
set -x
DIRECTORY="defendx*"
REPOSITORY="https://github.com/conzex/agent"
REFERENCE=""
OUT_NAME=""
CHECKSUM="no"
PKG_NAME=""
HAVE_PKG_NAME_WIN=false
HAVE_PKG_NAME_MAC=false
HAVE_PKG_NAME_LINUX=false
AWS_REGION="us-east-1"
KEYPATH="/etc/defendx"
WPKCERT="${KEYPATH}/wpkcert.pem"
WPKKEY="${KEYPATH}/wpkcert.key"
OUTDIR="/var/local/defendx"
CHECKSUMDIR="/var/local/checksum"

help() {
    set +x
    echo
    echo "Usage: ${0} [OPTIONS]"
    echo "It is required to use -k or --aws-wpk-key, --aws-wpk-cert parameters"
    echo
    echo "    -b,   --branch <branch>      [Required] Select Git branch or tag e.g. main"
    echo "    -o,   --output <name>        [Required] Name to the output package."
    echo "    -pn,  --package-name <name>  [Required] Path to package file (rpm, deb, apk, msi, pkg) to pack in wpk."
    echo "    -c,   --checksum             [Optional] Whether Generate checksum or not."
    echo "    --aws-wpk-key                [Optional] AWS Secrets manager Name/ARN to get WPK private key."
    echo "    --aws-wpk-cert               [Optional] AWS secrets manager Name/ARN to get WPK certificate."
    echo "    --aws-wpk-key-region         [Optional] AWS Region where secrets are stored."
    echo "    -h,   --help                 Show this help."
    echo
    exit ${1}
}

main() {
    while [ -n "${1}" ]
    do
        case "${1}" in
        "-b"|"--branch")
            if [ -n "${2}" ]; then
                REFERENCE="${2}"
                shift 2
            else
                echo "ERROR: Missing branch."
                help 1
            fi
            ;;
        "-o"|"--output")
            if [ -n "${2}" ]; then
                OUT_NAME="${2}"
                shift 2
            else
                echo "ERROR: Missing output name."
                help 1
            fi
            ;;
        "-pn"|"--package-name")
            if [ -n "${2}" ]; then
                PKG_NAME="${2}"
                case "${PKG_NAME: -4}" in
                    ".msi") HAVE_PKG_NAME_WIN=true ;;
                    ".pkg") HAVE_PKG_NAME_MAC=true ;;
                    ".rpm"|".deb"|".apk") HAVE_PKG_NAME_LINUX=true ;;
                    *)
                        echo "ERROR: Missing package file."
                        help 1
                        ;;
                esac
                shift 2
            fi
            ;;
        "-c"|"--checksum")
            CHECKSUM="yes"
            shift 1
            ;;
        "--aws-wpk-key")
            if [ -n "${2}" ]; then
                AWS_WPK_KEY="${2}"
                shift 2
            fi
            ;;
        "--aws-wpk-cert")
            if [ -n "${2}" ]; then
                AWS_WPK_CERT="${2}"
                shift 2
            fi
            ;;
        "--aws-wpk-key-region")
            if [ -n "${2}" ]; then
                AWS_REGION="${2}"
                shift 2
            fi
          ;;
        "-h"|"--help")
            help 0
            ;;
        *)
            help 1
        esac
    done

    mkdir -p ${KEYPATH}
    if [ -n "${AWS_WPK_CERT}" ] && [ -n "${AWS_WPK_KEY}" ]; then
        aws --region=${AWS_REGION} secretsmanager get-secret-value --secret-id ${AWS_WPK_CERT} | jq . > wpkcert.pem.json
        jq .SecretString wpkcert.pem.json | tr -d '"' | sed 's|\\n|\n|g' > ${WPKCERT}
        rm -f wpkcert.pem.json
        aws --region=${AWS_REGION} secretsmanager get-secret-value --secret-id ${AWS_WPK_KEY} | jq . > wpkcert.key.json
        jq .SecretString wpkcert.key.json | tr -d '"' | sed 's|\\n|\n|g' > ${WPKKEY}
        rm -f wpkcert.key.json
    fi

    curl -sL ${REPOSITORY}/tarball/${REFERENCE} | tar zx
    cd ${DIRECTORY}

    OUTPUT="${OUTDIR}/${OUT_NAME}"
    mkdir -p ${OUTDIR}

    if [ "${HAVE_PKG_NAME_WIN}" == true ]; then
        wpkpack ${OUTPUT} ${WPKCERT} ${WPKKEY} ${PKG_NAME} upgrade.bat do_upgrade.ps1
    elif [ "${HAVE_PKG_NAME_MAC}" == true ] || [ "${HAVE_PKG_NAME_LINUX}" == true ]; then
        wpkpack ${OUTPUT} ${WPKCERT} ${WPKKEY} ${PKG_NAME} upgrade.sh pkg_installer.sh
    else
        echo "ERROR: A package (MSI/PKG/RPM/DEB) is needed to build the WPK"
        help 1
    fi

    echo "PACKED FILE -> ${OUTPUT}"
    cd ${OUTDIR}

    if [[ ${CHECKSUM} == "yes" ]]; then
        mkdir -p ${CHECKSUMDIR}
        sha512sum "${OUT_NAME}" > "${CHECKSUMDIR}/${OUT_NAME}.sha512"
    fi
}

if [ "${BASH_SOURCE[0]}" = "${0}" ]
then
    main "$@"
fi
