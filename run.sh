source compile.sh

BUILD_DIR="release"

cd $BUILD_DIR
make
cd ..
$BUILD_DIR/llvmassignment $*