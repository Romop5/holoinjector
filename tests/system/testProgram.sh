SCRIPT_DIR=`dirname $0`
COMMAND_NAME=$@
TOKEN=`basename $1 | tr ' ' '_' `
ENHANCER_LIB=`realpath ${SCRIPT_DIR}/../../build/librepeater.so`
OUTPUT_IMAGE_DIR="${SCRIPT_DIR}/images"


mkdir -p "${OUTPUT_IMAGE_DIR}"

echo "Verifying command ${COMMAND_NAME}"
echo "Using ENHANCER_LIB: ${ENHANCER_LIB}"

###############################################################################
# 1. Test that when running application with enhancer, the original view is
# not corrupted => both screenshots should match pixel-by-pixel
###############################################################################
ENHANCER_NONINTRUSIVE=1\
    ENHANCER_SCREENSHOT=${OUTPUT_IMAGE_DIR}/${TOKEN}_original.bmp\
    ENHANCER_EXIT_AFTER=20\
    LD_PRELOAD=${ENHANCER_LIB} ${COMMAND_NAME}

ENHANCER_NOW=1\
    ENHANCER_CAMERAID=4\
    ENHANCER_ANGLE=1\
    ENHANCER_DISTANCE=1\
    ENHANCER_SCREENSHOT=${OUTPUT_IMAGE_DIR}/${TOKEN}_repeater.bmp\
    ENHANCER_EXIT_AFTER=20 LD_PRELOAD=${ENHANCER_LIB} ${COMMAND_NAME}

###############################################################################
# Compare both resutls
###############################################################################
