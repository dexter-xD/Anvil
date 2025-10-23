#!/bin/bash

# build debian package using cmake and cpack
set -e

# package configuration
VERSION="1.1.2"
PACKAGE_NAME="anvil"
PACKAGE_FILE="${PACKAGE_NAME}_${VERSION}_amd64.deb"

echo "Building ${PACKAGE_NAME} v${VERSION} Debian Package with CMake 🔥"
echo ""

# clean up previous builds
echo "Cleaning previous builds..."
rm -rf build/
rm -f ${PACKAGE_NAME}_*.deb

# create build directory
mkdir -p build
cd build

# configure with cmake
echo "Configuring with CMake..."
cmake -DCMAKE_BUILD_TYPE=Release -DCOPY_TO_EXAMPLE=OFF -DPACKAGE_NAME="${PACKAGE_NAME}" -DPACKAGE_VERSION="${VERSION}" ..

# build the project
echo "Building project..."
make

# create the debian package
echo "Creating Debian package..."
cpack

# move package to root directory
echo "Moving package..."
mv ${PACKAGE_NAME}_*.deb ../

cd ..

echo ""
echo "Package created successfully!"
echo "Package: ${PACKAGE_FILE}"
echo ""
echo "To install: sudo dpkg -i ${PACKAGE_FILE}"
echo "To remove: sudo apt remove ${PACKAGE_NAME}"
echo ""
echo "To test: dpkg-deb -I ${PACKAGE_FILE}"