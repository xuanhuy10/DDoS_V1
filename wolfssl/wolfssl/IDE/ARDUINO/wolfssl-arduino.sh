#!/bin/sh

# this script will reformat the wolfSSL source code to be compatible with
# an Arduino project
# run as bash ./wolfssl-arduino.sh [INSTALL] [path]
#
# ./wolfssl-arduino.sh
# The default is to install to a local wolfSSL directory (`ROOT_DIR`).
# If successfully built, and the INSTALL option is used, tis directory
# is then moved to the target.
#
# ./wolfssl-arduino.sh INSTALL
# Creates a local wolfSSL directory and then moves it to the ARDUINO_ROOT
#
# ./wolfssl-arduino.sh INSTALL /mnt/c/workspace/Arduino-wolfSSL-$USER
# Updates the Arduino-wolfSSL fork for $USER to refresh versions.
#
# To ensure a pristine build, the directory must not exist.
#
# Reminder there's typically no $USER for GitHub actions, but:
# ROOT_DIR="/mnt/c/Users/$USER/Documents/Arduino/libraries"
#
# The company name is "wolfSSL Inc."; There's a space, no comma, and a period after "Inc."
# The Arduino library name is "wolfssl" (all lower case)
# The Arduino library directory name is "wolfssl" (all lower case)
# The Arduino library include file is "wolfssl.h" (all lower case)
# The Published wolfSSL Arduino Registry is at https://github.com/wolfSSL/Arduino-wolfSSL.git
# See https://downloads.arduino.cc/libraries/logs/github.com/wolfSSL/Arduino-wolfSSL/
#
echo "wolfssl-arduino.sh v5.8.2 rev B"

ROOT_DIR="/wolfssl"

# The Arduino Version will initially have a suffix appended during fine tuning stage.
WOLFSSL_VERSION_ARUINO_SUFFIX=""

# For verbose copy, set CP_CMD="-v", otherwise clear it: CP_CMD="cp"
# Do not set to empty string, as copy will fail with this: CP_CMD=""
# CP_CMD="cp -v "
CP_CMD="cp "

# Specify the executable shell checker you want to use:
MY_SHELLCHECK="shellcheck"

# There are special circumstances to publish to GitHub repository.
# Typically: https://github.com/wolfSSL/Arduino-wolfSSL
#
# Unlike a local Arduino library that requires a clean directory,
# we'll allow extra files, overwrites, etc.
#
# Note in all cases, the local IDE/ARDUINO/wolfssl must be empty.
THIS_INSTALL_IS_GITHUB="false"

# Check if the executable is available in the PATH
if command -v "$MY_SHELLCHECK" >/dev/null 2>&1; then
    # Run your command here
    shellcheck "$0" || exit 1
else
    echo "$MY_SHELLCHECK is not installed. Please install it if changes to this script have been made."
fi

if ! [ "$CP_CMD" = "cp " ]; then
    if [ "$CP_CMD" = "cp -v" ]; then
        echo "Copy verbose mode"
    else
        echo "ERROR: Copy mode not supported: $CP_CMD"
        exit 1
    fi
fi

if [ "$ROOT_DIR" = "" ]; then
    echo "ERROR: ROOT_DIR cannot be blank"
    exit 1
fi

if [ "$ARDUINO_ROOT" = "" ]; then
    echo "No ARDUINO_ROOT export... detecting..."
    ARDUINO_ROOT="$HOME/Arduino/libraries"

    # Check environment
    if [ -n "$WSL_DISTRO_NAME" ]; then
        # we found a non-blank WSL environment distro name
        echo "Found WSL: $WSL_DISTRO_NAME"
        current_path="$(pwd)"
        pattern="/mnt/?"
        if echo "$current_path" | grep -Eq "^$pattern"; then
            # if we are in WSL and shared Windows file system, 'ln' does not work.
            ARDUINO_ROOT="/mnt/c/Users/$USER/Documents/Arduino/libraries"
            echo "ARDUINO_ROOT set to $ARDUINO_ROOT"
        fi
    fi
else
    echo "Using export ARDUINO_ROOT"
fi
echo "The Arduino library root is: $ARDUINO_ROOT"

if [ $# -gt 0 ]; then
    THIS_OPERATION="$1"
    if [ "$THIS_OPERATION" = "INSTALL" ]; then
        THIS_INSTALL_DIR=$2

        if [ "$THIS_INSTALL_DIR" = "/" ]; then
            echo "ERROR: THIS_INSTALL_DIR cannot be /"
            exit 1
        fi

        echo "Install is active."

        if [ "$THIS_INSTALL_DIR" = "" ]; then
            if [ -d "$ARDUINO_ROOT$ROOT_DIR" ]; then
                echo "Error: the installation directory already exists: $ARDUINO_ROOT$ROOT_DIR"
                echo "A new directory needs to be created to ensure there are no stray files"
                echo "Please delete or move the directory and try again."
                exit 1
            fi
        else
            echo "Installing to $THIS_INSTALL_DIR"
            if [ -d "$THIS_INSTALL_DIR/.git" ];then
                echo "Target is a GitHub root repository."
                THIS_INSTALL_IS_GITHUB="true"
            else
                echo "Target is NOT a GitHub root directory repository. (e.g. not wolfssl/Arduino-wolfssl)"
            fi
        fi
    else
        echo "Error: not a valid operation: $THIS_OPERATION"
        exit 1
    fi
else
    echo "INSTALL parameter not specified. Installing to ROOT_DIR=$ROOT_DIR"
fi


ROOT_SRC_DIR="${ROOT_DIR}/src"
EXAMPLES_DIR="${ROOT_DIR}/examples"

if [ -n "$WOLFSSL_EXAMPLES_ROOT" ]; then
    EXTRA_EXAMPLES_DIR="${WOLFSSL_EXAMPLES_ROOT}/Arduino"
    echo "EXTRA_EXAMPLES_DIR=$EXTRA_EXAMPLES_DIR"
else
    echo "There are additional examples at https://github.com/wolfSSL/wolfssl-examples"
    echo "Set WOLFSSL_EXAMPLES_ROOT to your local directory to include those examples."
fi
WOLFSSL_SRC="${ROOT_SRC_DIR}/src"
WOLFSSL_HEADERS="${ROOT_SRC_DIR}/wolfssl"
WOLFCRYPT_ROOT="${ROOT_SRC_DIR}/wolfcrypt"
WOLFCRYPT_SRC="${WOLFCRYPT_ROOT}/src"
WOLFCRYPT_HEADERS="${WOLFSSL_HEADERS}/wolfcrypt"
OPENSSL_DIR="${WOLFSSL_HEADERS}/openssl"


# TOP indicates the file directory for top level of the wolfssl repository.
TOP_DIR="../.."
WOLFSSL_SRC_TOP="${TOP_DIR}/src"
WOLFSSL_HEADERS_TOP="${TOP_DIR}/wolfssl"
WOLFCRYPT_ROOT_TOP="${TOP_DIR}/wolfcrypt"
WOLFCRYPT_SRC_TOP="${WOLFCRYPT_ROOT_TOP}/src"
WOLFCRYPT_HEADERS_TOP="${WOLFSSL_HEADERS_TOP}/wolfcrypt"
OPENSSL_DIR_TOP="${WOLFSSL_HEADERS_TOP}/openssl"


WOLFSSL_VERSION=$(grep -i "LIBWOLFSSL_VERSION_STRING" ${TOP_DIR}/wolfssl/version.h | cut -d '"' -f 2)
if [ "$WOLFSSL_VERSION" = "" ]; then
    echo "Current user: [$USER]"
    if [ "$USER" = "" ] || [ "$USER" = "runner" ]; then
        # Typically when there's no user, it is a GitHub workflow. It is not guaranteed to be "runner"
        echo "No USER found, no version.h found. Setting Version text to [GitHub] for assumed workflow."
        WOLFSSL_VERSION="GitHub"
    else
        echo "ERROR: Could not find wolfSSL Version in ${TOP_DIR}/wolfssl/version.h"
        echo "Check autogen.sh and configure"
        exit 1
    fi
else
    echo "Found wolfSSL version $WOLFSSL_VERSION"
    echo "# WOLFSSL_VERSION_ARUINO_SUFFIX $WOLFSSL_VERSION_ARUINO_SUFFIX"
fi
echo ""

THIS_DIR=${PWD##*/}

if [ "$THIS_DIR" = "ARDUINO" ]; then
    # mkdir ./wolfssl
    if [ -d ".${ROOT_DIR}" ]; then
        echo "ERROR: $(realpath ".${ROOT_DIR}") is not empty; failed prior install? Please remove."
        exit 1
    else
        echo "Step 01: mkdir .${ROOT_DIR}"
        mkdir ."${ROOT_DIR}"
    fi

    # mkdir ./wolfssl/src
    if [ ! -d ".${ROOT_SRC_DIR}" ]; then
        echo "Step 02: mkdir .${ROOT_SRC_DIR}"
        mkdir ."${ROOT_SRC_DIR}"
    fi

    # mkdir ./wolfssl/src/wolfssl
    if [ ! -d ".${WOLFSSL_HEADERS}" ]; then
        echo "Step 03: mkdir .${WOLFSSL_HEADERS}"
        mkdir ."${WOLFSSL_HEADERS}"
    fi

    #  cp ../../wolfssl/*.h  ./wolfssl/src/wolfssl
    echo "Step 04: cp    ${WOLFSSL_HEADERS_TOP}/*.h              .${WOLFSSL_HEADERS}"
    $CP_CMD "${WOLFSSL_HEADERS_TOP}"/*.h ."${WOLFSSL_HEADERS}"
    if [ ! -d ".${WOLFCRYPT_HEADERS}" ]; then
        #  mkdir ./wolfssl/src/wolfssl/wolfcrypt
        echo "Step 05: mkdir .${WOLFCRYPT_HEADERS}"
        mkdir ."${WOLFCRYPT_HEADERS}"
        mkdir ."${WOLFCRYPT_HEADERS}/port"
        mkdir ."${WOLFCRYPT_HEADERS}/port/atmel"
        mkdir ."${WOLFCRYPT_HEADERS}/port/Espressif"
    fi

    # cp  ../../wolfssl/wolfcrypt/*.h  ./wolfssl/src/wolfssl/wolfcrypt
    echo "Step 06: cp    ${WOLFCRYPT_HEADERS_TOP}/*.h    .${WOLFCRYPT_HEADERS}"
    $CP_CMD "${WOLFCRYPT_HEADERS_TOP}"/*.h                ."${WOLFCRYPT_HEADERS}"                 || exit 1
    $CP_CMD "${WOLFCRYPT_HEADERS_TOP}"/port/atmel/*.h     ."${WOLFCRYPT_HEADERS}/port/atmel"      || exit 1
    $CP_CMD "${WOLFCRYPT_HEADERS_TOP}"/port/Espressif/*.h ."${WOLFCRYPT_HEADERS}/port/Espressif"  || exit 1

    # Add in source files to wolfcrypt/src
    if [ ! -d ".${WOLFCRYPT_ROOT}" ]; then
        # mkdir ./wolfssl/src/wolfcrypt
        echo "Step 07: mkdir .${WOLFCRYPT_ROOT}"
        mkdir ."${WOLFCRYPT_ROOT}"
    fi

    # mkdir ./wolfssl/src/wolfcrypt/src
    if [ ! -d ".${WOLFCRYPT_SRC}" ]; then
        echo "Step 08: mkdir .${WOLFCRYPT_SRC}"
        mkdir ."${WOLFCRYPT_SRC}"
        mkdir ."${WOLFCRYPT_SRC}"/port
        mkdir ."${WOLFCRYPT_SRC}"/port/atmel
        mkdir ."${WOLFCRYPT_SRC}"/port/Espressif
    fi

    # cp  ../../wolfcrypt/src/*.c  ./wolfssl/src/wolfcrypt/src
    echo "Step 09: cp    ${WOLFCRYPT_SRC_TOP}/*.c        .${WOLFCRYPT_SRC}"
    $CP_CMD -r "${WOLFCRYPT_SRC_TOP}"/*.c                  ."${WOLFCRYPT_SRC}"                || exit 1
    $CP_CMD -r "${WOLFCRYPT_SRC_TOP}"/port/atmel/*.c       ."${WOLFCRYPT_SRC}"/port/atmel     || exit 1
    $CP_CMD -r "${WOLFCRYPT_SRC_TOP}"/port/Espressif/*.c   ."${WOLFCRYPT_SRC}"/port/Espressif || exit 1

    # Add in source files to top level src folders
    if [ ! -d ".${WOLFSSL_SRC}" ]; then
        # mkdir ./wolfssl/src/src
        echo "Step 10: mkdir .${WOLFSSL_SRC}"
        mkdir ."${WOLFSSL_SRC}"
    fi
    $CP_CMD "${WOLFSSL_SRC_TOP}"/*.c ."${WOLFSSL_SRC}"                                        || exit 1
    # put bio and evp as includes
    $CP_CMD ."${WOLFSSL_SRC}"/bio.c   ."${WOLFSSL_HEADERS}"                                   || exit 1
    $CP_CMD ."${WOLFCRYPT_SRC}"/evp.c ."${WOLFSSL_HEADERS}"                                   || exit 1

    # make a copy of evp.c and bio.c for ssl.c to include inline
    $CP_CMD ."${WOLFSSL_HEADERS}"/evp.c ."${WOLFCRYPT_SRC}"/evp.c                             || exit 1
    $CP_CMD ."${WOLFSSL_HEADERS}"/bio.c ."${WOLFCRYPT_SRC}"/bio.c                             || exit 1

    # copy openssl compatibility headers to their appropriate location
    if [ ! -d ".${OPENSSL_DIR}" ]; then
        mkdir ."${OPENSSL_DIR}"
    fi
    $CP_CMD "${OPENSSL_DIR_TOP}"/* ."${OPENSSL_DIR}"                                          || exit 1

    # Finally, copy the Arduino-specific wolfssl library files into place: [lib]/src
    $CP_CMD ./wolfssl.h           ".${ROOT_SRC_DIR}"/wolfssl.h           || exit 1
    $CP_CMD ./wolfssl-arduino.cpp ".${ROOT_SRC_DIR}"/wolfssl-arduino.cpp || exit 1

    unset NO_ARDUINO_EXAMPLES
    echo "Copy examples...."
    # Copy examples
    mkdir -p ".${ROOT_SRC_DIR}"/examples

    EXAMPLES_DIR_REAL_PATH=$(realpath ".${EXAMPLES_DIR}")
    echo "Source WOLFSSL_EXAMPLES_ROOT=$WOLFSSL_EXAMPLES_ROOT"
    echo "Destination EXAMPLES_DIR=.${EXAMPLES_DIR}"
    echo "EXAMPLES_DIR_REAL_PATH=${EXAMPLES_DIR_REAL_PATH}"

    # Only explicit source code is copied to the Arduino library. Edit with caution, no automation:
    if [ -n "$WOLFSSL_EXAMPLES_ROOT" ]; then
        echo "Copy template example...."
        mkdir -p ".${EXAMPLES_DIR}"/template/wolfssl_library/src
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/template/template.ino                             ".${EXAMPLES_DIR}"/template/template.ino                             || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/template/README.md                                ".${EXAMPLES_DIR}"/template/README.md                                || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/template/wolfssl_helper.c                         ".${EXAMPLES_DIR}"/template/wolfssl_helper.c                         || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/template/wolfssl_helper.h                         ".${EXAMPLES_DIR}"/template/wolfssl_helper.h                         || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/template/wolfssl_library/wolfssl_library.h        ".${EXAMPLES_DIR}"/template/wolfssl_library/wolfssl_library.h        || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/template/wolfssl_library/src/wolfssl_library.cpp  ".${EXAMPLES_DIR}"/template/wolfssl_library/src/wolfssl_library.cpp  || exit 1

        echo "Copy wolfssl_AES_CTR example...."
        mkdir -p ".${EXAMPLES_DIR}"/wolfssl_AES_CTR
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_AES_CTR/wolfssl_AES_CTR.ino               ".${EXAMPLES_DIR}"/wolfssl_AES_CTR/wolfssl_AES_CTR.ino               || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_AES_CTR/README.md                         ".${EXAMPLES_DIR}"/wolfssl_AES_CTR/README.md                         || exit 1

        echo "Copy wolfssl_client example...."
        mkdir -p ".${EXAMPLES_DIR}"/wolfssl_client
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_client/wolfssl_client.ino                 ".${EXAMPLES_DIR}"/wolfssl_client/wolfssl_client.ino                 || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_client/README.md                          ".${EXAMPLES_DIR}"/wolfssl_client/README.md                          || exit 1

        echo "Copy wolfssl_client_dtls example...."
        mkdir -p ".${EXAMPLES_DIR}"/wolfssl_client_dtls
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_client_dtls/wolfssl_client_dtls.ino       ".${EXAMPLES_DIR}"/wolfssl_client_dtls/wolfssl_client_dtls.ino       || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_client_dtls/README.md                     ".${EXAMPLES_DIR}"/wolfssl_client_dtls/README.md                     || exit 1

        echo "Copy wolfssl_server example...."
        mkdir -p .${EXAMPLES_DIR}/wolfssl_server
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_server/wolfssl_server.ino                 ".${EXAMPLES_DIR}"/wolfssl_server/wolfssl_server.ino                 || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_server/README.md                          ".${EXAMPLES_DIR}"/wolfssl_server/README.md                          || exit 1

        echo "Copy wolfssl_server_dtls example...."
        mkdir -p .${EXAMPLES_DIR}/wolfssl_server_dtls
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_server_dtls/wolfssl_server_dtls.ino       ".${EXAMPLES_DIR}"/wolfssl_server_dtls/wolfssl_server_dtls.ino       || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_server_dtls/README.md                     ".${EXAMPLES_DIR}"/wolfssl_server_dtls/README.md                     || exit 1

        echo "Copy wolfssl_version example...."
        mkdir -p .${EXAMPLES_DIR}/wolfssl_version
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_version/wolfssl_version.ino               ".${EXAMPLES_DIR}"/wolfssl_version/wolfssl_version.ino               || exit 1
        $CP_CMD "$WOLFSSL_EXAMPLES_ROOT"/Arduino/sketches/wolfssl_version/README.md                         ".${EXAMPLES_DIR}"/wolfssl_version/README.md                         || exit 1
    else
        NO_ARDUINO_EXAMPLES=1
    fi
    echo "Examples copied to .${EXAMPLES_DIR}"
    echo "ls ${EXAMPLES_DIR_REAL_PATH}"
    ls "${EXAMPLES_DIR_REAL_PATH}"
else
    echo "ERROR: You must be in the IDE/ARDUINO directory to run this script"
    exit 1
fi

# At this point, the library is complete, but we need some additional files.
#
# optional diagnostics:
# echo ".${ROOT_DIR}"
# echo "${TOP_DIR}"
# echo "cp ${TOP_DIR}/README.md     .${ROOT_DIR}/"

# Replace the `${WOLFSSL_VERSION}` text in Arduino_README_prepend.md,
# saving it to a .tmp file. Prepend that file to the wolfSSL README.md
# file as PREPENDED_README.md, then copy that to the publish directory
# as an Arduino-specific README.md file.
VERSION_PLACEHOLDER="\${WOLFSSL_VERSION}"
ARDUINO_VERSION_SUFFIX_PLACEHOLDER="\${WOLFSSL_VERSION_ARUINO_SUFFIX}"

# This is the SOURCE to prepend. Note the OUTPUT is PREPENDED_README.md later copied to README.md
PREPEND_FILE="Arduino_README_prepend.md"
PROPERTIES_FILE_TEMPLATE="library.properties.template"
sed s/"$VERSION_PLACEHOLDER"/"$WOLFSSL_VERSION"/ "$PREPEND_FILE" > "$PREPEND_FILE.tmp"
cat "$PREPEND_FILE.tmp" ${TOP_DIR}/README.md > PREPENDED_README.md

# Here we'll insert the wolfSSL version into the `library.properties.tmp` file, along with an Arduino version suffix.
# The result should be something like version=5.6.6.Arduino.1 (for the 1st incremental version on top of 5.6.6)
sed            s/"$VERSION_PLACEHOLDER"/"$WOLFSSL_VERSION"/                              "$PROPERTIES_FILE_TEMPLATE" > "library.properties.tmp"
sed -i.backup  s/"$ARDUINO_VERSION_SUFFIX_PLACEHOLDER"/"$WOLFSSL_VERSION_ARUINO_SUFFIX"/ "library.properties.tmp"

# cat library.properties.tmp
# echo "${WOLFSSL_VERSION_ARUINO_SUFFIX}"

echo "Step 11: Final root file copy"
$CP_CMD  PREPENDED_README.md          ."${ROOT_DIR}"/README.md           || exit 1
$CP_CMD  library.properties.tmp       ."${ROOT_DIR}"/library.properties  || exit 1
$CP_CMD  "${TOP_DIR}"/"LICENSING"     ."${ROOT_DIR}"/                    || exit 1
$CP_CMD  "${TOP_DIR}"/"README"        ."${ROOT_DIR}"/                    || exit 1
$CP_CMD  "${TOP_DIR}"/"COPYING"       ."${ROOT_DIR}"/                    || exit 1
$CP_CMD  "${TOP_DIR}"/"ChangeLog.md"  ."${ROOT_DIR}"/                    || exit 1
$CP_CMD  "${TOP_DIR}"/".editorconfig" ."${ROOT_DIR}"/                    || exit 1
$CP_CMD  "${TOP_DIR}"/".gitignore"    ."${ROOT_DIR}"/                    || exit 1

$CP_CMD  "keywords.txt"               ."${ROOT_DIR}"/                    || exit 1


echo "Step 12: Workspace to publish:"
echo ""
head -n 3  PREPENDED_README.md
echo ""
ls ./wolfssl -al
echo ""

# Optionally install to a separate directory.
# Note we should have exited above if a problem was encountered,
# as we'll never want to install a bad library.
if [ "$THIS_OPERATION" = "INSTALL" ]; then
    echo "Config:"
    echo "cp ../../examples/configs/user_settings_arduino.h  ".${ROOT_SRC_DIR}"/user_settings.h"
    # Nearly an ordinary copy, but we remove any lines with ">>" (typically edit with caution warning in comments)
    grep -v '>>' ../../examples/configs/user_settings_arduino.h > ".${ROOT_SRC_DIR}"/user_settings.h || exit 1

    echo "This user_settings.h revision string:"
    grep "WOLFSSL_USER_SETTINGS_ID" ."${ROOT_SRC_DIR}/user_settings.h"
    echo ""

    if [ "$THIS_INSTALL_IS_GITHUB" = "true" ]; then
        echo "Installing to GitHub directory: $THIS_INSTALL_DIR"
        cp -r ."$ROOT_DIR"/* "$THIS_INSTALL_DIR"                || exit 1
        echo "Removing workspace library directory: .$ROOT_DIR"
        rm -rf ".$ROOT_DIR"                                     || exit 1
    else

        echo "Installing to local directory:"
        if [ "$THIS_INSTALL_DIR" = "" ]; then
            if [ -n "$WSL_DISTRO_NAME" ]; then
                # setfattr not installed by default
                # echo "Set system.wsl_case_sensitive .$ROOT_DIR"
                # setfattr -x system.wsl_case_sensitive .$ROOT_DIR
                #
                # use copy instead of move to avoid possible system.wsl_case_sensitive warnings
                echo "cp -r  .\"$ROOT_DIR\" \"$ARDUINO_ROOT\""
                      cp -r   ."$ROOT_DIR"   "$ARDUINO_ROOT"    || exit 1

                echo "rm -rf .\"$ROOT_DIR\""
                      rm -rf  ."$ROOT_DIR"                      || exit 1
            else
                echo "mv  .$ROOT_DIR   $ARDUINO_ROOT"
                      mv ."$ROOT_DIR" "$ARDUINO_ROOT"           || exit 1
            fi
            echo "Arduino wolfSSL Version: $WOLFSSL_VERSION$WOLFSSL_VERSION_ARUINO_SUFFIX"
        else
            echo "cp -r .\"$ROOT_DIR\"/* \"$THIS_INSTALL_DIR\""
            mkdir -p "$THIS_INSTALL_DIR"                        || exit 1
            cp -r ."$ROOT_DIR"/* "$THIS_INSTALL_DIR"            || exit 1
        fi
    fi
fi

if [ -n "$NO_ARDUINO_EXAMPLES" ]; then
    echo ""
    echo "WARNING: No examples copied. Set WOLFSSL_EXAMPLES_ROOT as appropriate."
    echo ""
fi
echo "Done!"
