TESTDIR=`pwd`
TEST_EXE=`realpath testProgram.sh`
LOGL_DIR="/home/roman/hobby/LearnOpenGL/"

function testExample()
{
    # Enter LearnOpenGL directory
    cd ${LOGL_DIR} 
    # Store one of "src/X/Y" path
    WORKDIR=$1
    # Get Y from param
    WORKDIR_BASENAME=`basename $1`
    BIN_DIR=`realpath bin`
    EXE=`find ${BIN_DIR} | grep "${WORKDIR_BASENAME}" | grep "__"`

    cd ${WORKDIR}
    
    ${TEST_EXE} ${EXE}

    # Restore original directory
    cd ${TESTDIR}

}


testExample "src/3.model_loading/1.model_loading"
testExample "src/5.advanced_lighting/9.ssao"

testExample "src/5.advanced_lighting/6.hdr"
testExample "src/5.advanced_lighting/3.1.3.shadow_mapping"
testExample "src/5.advanced_lighting/1.advanced_lighting"
testExample "src/5.advanced_lighting/7.bloom"
testExample "src/5.advanced_lighting/3.2.2.point_shadows_soft"
testExample "src/5.advanced_lighting/4.normal_mapping"

testExample "src/4.advanced_opengl/6.1.cubemaps_skybox"
testExample "src/4.advanced_opengl/8.advanced_glsl_ubo"
testExample "src/4.advanced_opengl/5.1.framebuffers"
testExample "src/6.pbr/2.2.1.ibl_specular"
