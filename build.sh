#!/bin/bash

script_name=$0
compiler="gcc"
build_thread="36"
build_type="Release"

print_help() {
  cat << EOF
Usage: ${script_name} [OPTION]...
Build project.

Mandatory arguments to long options are mandatory for short options too.
  -h, --help                 display this help and exit.
  -c, --compiler <gcc|clang> set compiler(default: gcc).
  -j, --threads              use parallel build(default: 36).
  -b, --build-type           Release or Debug(default: Release)
EOF
  exit 1;
}

find . -name "*.sh" | xargs chmod +x

short=c:hs:j:b:
long=compiler:,help,threads:,build-type:
TEMP=`getopt -o ${short} --long ${long} -n ${script_name} -- "$@"`
eval set -- "$TEMP"
while true ; do
  case "$1" in
    -c|--compiler)
      compiler=$2
      echo "[INFO] build library using '${compiler}'" ;
      shift 2 ;;
    -j|--threads)
      build_threads=$2
      echo "[INFO] '${build_threads}' threads parallel to build" ;
      shift 2 ;;
    -b|--build-type)
      build_type=$2
      echo "[INFO] build type is '${build_type}'" ;
      shift 2 ;;
    -h|--help)
      print_help;
      shift ;;
    --) shift ;
      break ;;
    *) echo "[ERROR]" ; exit 1 ;;
  esac
done

exeIsValid() {
  if type $1 &> /dev/null;
  then
    return 0
  else
    echo "[ERROR] please install ${1} tools and set shell environment PATH to find it"
    exit 1
  fi
}

exeIsValid cmake
exeIsValid make

export PROJECT_ROOT=$(cd `dirname $0` && pwd)
build="build"
echo "[INFO] build project in ${PROJECT_ROOT}"
cd ${PROJECT_ROOT}
mkdir -p ${build}/${compiler}
mkdir -p install/${compiler}

echo "[INFO] android ndk: ${ANDROID_NDK}"
options=""
if [ "${compiler}" == "gcc" ] ; then
  exeIsValid gcc
  options="${options} \
    -DCMAKE_C_COMPILER=`which gcc` \
    -DCMAKE_CXX_COMPILER=`which g++` \
    -DCMAKE_BUILD_TYPE=${build_type} \
    -DCMAKE_STRIP=`which strip` "
fi

if [ "${compiler}" == "clang" ] ; then
  exeIsValid clang
  options="${options} \
    -DCMAKE_C_COMPILER=`which clang` \
    -DCMAKE_CXX_COMPILER=`which clang++` \
    -DCMAKE_BUILD_TYPE=${build_type} \
    -DCMAKE_STRIP=`which strip` "
fi

if [ "${compiler}" == "ndk" ] ; then
  options="${options} \
    -DCMAKE_TOOLCHAIN_FILE=${ANDROID_NDK}/build/cmake/android.toolchain.cmake \
    -DCMAKE_ARM_NEON=ON \
    -DOPT_WITH_NEON=ON \
    -DUSE_OPENMP=ON \
    -DANDROID_ABI="arm64-v8a" \
    -DANDROID_PLATFORM="android-16" \
    -DCMAKE_BUILD_TYPE=${build_type} \
    -DCMAKE_STRIP=`which strip` "
fi

cd ${PROJECT_ROOT}
cd ${build}/${compiler}
cmake ../../ -DCMAKE_INSTALL_PREFIX=${PROJECT_ROOT}/install/${compiler}/${build_type} ${options}
make -j${build_threads}
make install -j${build_threads} || exit 1

