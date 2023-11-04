#!/bin/bash

declare -a llvm_configs=("llvm-config-12"
                         "llvm-config-13"
                         "llvm-config-14"
                         "llvm-config-15"
                         "llvm-config-16"
                         "llvm-config-17")

for config_path in ${llvm_configs[@]}; do
  echo "---------------------------------------------------------"
  echo " Running clang-uml tests against LLVM $(${config_path} --version)"
  echo "---------------------------------------------------------"
  make clean
  LLVM_CONFIG_PATH=$config_path NUMPROC=12 make test
done