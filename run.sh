source sh/compile.sh

BUILD_DIR="release"
OUTPUT_DIR="output"

mkdir $OUTPUT_DIR
cd $BUILD_DIR
make
cd ..
$BUILD_DIR/anderson $*

dot -Tpng $OUTPUT_DIR/ptg.dot -o $OUTPUT_DIR/ptg.png
code $OUTPUT_DIR/*.png