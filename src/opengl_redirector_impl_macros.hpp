/**
 * Simple reflection library for defining entities with same-type attributes,
 * and having ability to enumerate these attributes when compiled.
 */
//-----------------------------------------------------------------------------
// ARG_COUNT determines what is the size of list
// e.g. ARG_COUNT(a,b) should return 2
#define ARG_COUNT_IMPL(a,b,c,d,e,f,h,i,j,k,l,m,n,o,p,r,s,t,v,u,w,z,...) z
#define ARG_COUNT(...) ARG_COUNT_IMPL(__VA_ARGS__,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)

// CONCAT allows to turn macro argument to part of indetifier
// e.g. in order to use EXPAND_INDEX, where INDEX is a macro, see below for usage
#define CAT(a, ...) a ## __VA_ARGS__
#define CONCAT(a, ...) CAT(a,__VA_ARGS__)

// Implementation of Finite-depth recursion
// C preprocessor doesn't support recursion, however,
// one can create depth up to N manually, and call corresponding starting
// level, for instance, with using ARG_COUNT to get the level
#define EXPAND_1(F, param, ...) F(param) 
#define EXPAND_2(F, param, ...) F(param) EXPAND_1(F,__VA_ARGS__)
#define EXPAND_3(F, param, ...) F(param) EXPAND_2(F,__VA_ARGS__)
#define EXPAND_4(F, param, ...) F(param) EXPAND_3(F,__VA_ARGS__)
#define EXPAND_5(F, param, ...) F(param) EXPAND_4(F,__VA_ARGS__)
#define EXPAND(F,...) CONCAT(EXPAND_,ARG_COUNT(__VA_ARGS__)) (F,__VA_ARGS__)

// Expand for binary operators
#define EXPANDB_0(...) 
// HACK: ARG_COUNT() returns 1 instead of 0 :(
#define EXPANDB_1(...) 
#define EXPANDB_2(F, paramA,paramB, ...) F(paramA,paramB)
#define EXPANDB_4(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_2(F,__VA_ARGS__)
#define EXPANDB_6(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_4(F,__VA_ARGS__)
#define EXPANDB_8(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_6(F,__VA_ARGS__)
#define EXPANDB_10(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_8(F,__VA_ARGS__)
#define EXPANDB_12(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_10(F,__VA_ARGS__)
#define EXPANDB_14(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_12(F,__VA_ARGS__)
#define EXPANDB_16(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_14(F,__VA_ARGS__)
#define EXPANDB_18(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_16(F,__VA_ARGS__)
#define EXPANDB_20(F, paramA,paramB, ...) F(paramA,paramB) EXPANDB_18(F,__VA_ARGS__)
#define EXPANDB(F,...) CONCAT(EXPANDB_,ARG_COUNT(__VA_ARGS__)) (F,__VA_ARGS__)



/*==================================================
 * HELPER MACROS - OpenGL
 *=================================================*/
#define OPENGL_LOG_API_CALL helper::log_api_call

/// Expands 'type, name' to ', type name '
#define OPENGL_EXPAND_PAIR(a,b) , a b
/// Expands a sequence of pairs into comma-separated 'type name' pairs
#define OPENGL_EXPAND_PROTOTYPE(a,b, ...) \
    a b EXPANDB(OPENGL_EXPAND_PAIR, __VA_ARGS__)


/// Expands 'type, name' to ', type name '
#define OPENGL_EXPAND_LOG_PAIR(a,b)  << ", " << std::to_string(b)
/// Expand (a,b,...) to a << ", " << b << ...
#define OPENGL_EXPAND_LOG(a,b, ...) \
    std::to_string(b) EXPANDB(OPENGL_EXPAND_LOG_PAIR, __VA_ARGS__)

#define OPENGL_EXPAND_NAME(a,b) , b
/// Expands a sequence of pairs into call list
#define OPENGL_EXPAND_ARGUMENTS(a,b, ...) \
    b EXPANDB(OPENGL_EXPAND_NAME,__VA_ARGS__)


/// Redirect glXYZ to OpenglRedirectorBase's method
#define OPENGL_REDIRECTOR_API(_retType, _name, ...)\
_retType impl##_name( OPENGL_EXPAND_PROTOTYPE(__VA_ARGS__) ) \
{ \
    OPENGL_LOG_API_CALL (""#_name); \
    return g_OpenGLRedirector-> _name(OPENGL_EXPAND_ARGUMENTS(__VA_ARGS__)); \
} \
static helper::RegisterAPIFunction register_impl##_name(""#_name, reinterpret_cast<void*>(&impl##_name));

#define OPENGL_REDIRECTOR_METHOD(_retType, _name, ...)\
_retType OpenglRedirectorBase :: _name ( OPENGL_EXPAND_PROTOTYPE(__VA_ARGS__) ) \
{ \
    using implType = _retType ( OPENGL_EXPAND_PROTOTYPE(__VA_ARGS__) ); \
    static auto original = getOriginalSymbolAddress(""#_name); \
    return reinterpret_cast<implType*>(original)(OPENGL_EXPAND_ARGUMENTS(__VA_ARGS__)); \
}


/*
 * @brief Define and forward proxy to implementation
 *
 * e.g. OPENGL_FORWARD(void, glClear, GLbitfield, mask); will be forwarded to:
 * GLAPI void GLAPIENTRY glClear( GLbitfield mask );
 */
#define OPENGL_FORWARD(_retType, _name, ...)\
    OPENGL_REDIRECTOR_API(_retType, _name, __VA_ARGS__) \
    OPENGL_REDIRECTOR_METHOD(_retType, _name, __VA_ARGS__) \


/*
 * @brief Define own handler
 *
 * one could use FORWARD object to forward original parameters or ORIGINAL
 */
#define OPENGL_REDEFINE(_retType, _name, ...)\
extern "C" _retType _name ( OPENGL_EXPAND_PROTOTYPE(__VA_ARGS__) ) \
{ \
    using implType = _retType ( OPENGL_EXPAND_PROTOTYPE(__VA_ARGS__) ); \
    static auto _Symbol = hooking::SymbolReference(libglPath, ""#_name); \
    auto ORIGINAL = reinterpret_cast<implType*>(_Symbol()); \
    auto FORWARD = std::bind(ORIGINAL, OPENGL_EXPAND_ARGUMENTS(__VA_ARGS__) ); \

