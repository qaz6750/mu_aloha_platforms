#!/bin/bash
# Assume building on ubuntu 22.04
sudo apt update

# Install Dependencies
sudo apt -y install uuid-dev clang llvm gcc-aarch64-linux-gnu lld pip git mono-devel build-essential iasl nasm python3 python3-git python3-pip gettext locales gnupg ca-certificates python3-venv git-core curl

# Install nuget
sudo curl -o /usr/local/bin/nuget.exe https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
alias nuget="mono /usr/local/bin/nuget.exe"

#cargo install --force cargo-make
#cargo add cargo-tarpaulin

export CLANGPDB_BIN=/usr/lib/llvm-38/bin/ && export CLANGPDB_AARCH64_PREFIX=aarch64-linux-gnu-
