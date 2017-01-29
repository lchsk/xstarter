#!/bin/sh

echo "Cleaning xstarter directory..."
rm -f Makefile
rm -f CMakeCache.txt
rm -f cmake_install.cmake
rm -f cmake_uninstall.cmake
rm -f install_manifest.txt
rm -f nohup.out
rm -rf CMakeFiles
rm -rf ./bin
rm -rf _CPack_Packages
rm -f xstarter-*.deb
rm -f xstarter-*.tar.gz
rm -f xstarter-*.zip
rm -f CPackSourceConfig.cmake CPackConfig.cmake

echo "Cleaning tests..."

cd tests; \
rm -f Makefile; \
rm -f CMakeCache.txt; \
rm -rf CMakeFiles; \
rm -rf cmake_install.cmake; \
rm -rf xstarter_tests
