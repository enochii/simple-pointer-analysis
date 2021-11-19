source sh/compile.sh

BUILD_DIR="release"
OUTPUT_DIR="output"

rm -rf $OUTPUT_DIR
mkdir $OUTPUT_DIR
cd $BUILD_DIR
make
cd ..
$BUILD_DIR/anderson $*

dot -Tpng $OUTPUT_DIR/ptg.dot -o $OUTPUT_DIR/ptg.png
code $OUTPUT_DIR/*.png