
#define LANG_NAME_LOWER glsl
#define LANG_NAME_CAMEL Glsl

#include "lexer_generator/4coder_lex_gen_main.cpp"

internal void
build_language_model(void){
    u8 utf8[129];
    smh_utf8_fill(utf8);
    
    smh_set_base_character_names();
    smh_typical_tokens();
    
    // CPP Names
    sm_char_name('!', "Not");
    sm_char_name('&', "And");
    sm_char_name('|', "Or");
    sm_char_name('%', "Mod");
    sm_char_name('^', "Xor");
    sm_char_name('?', "Ternary");
    sm_char_name('/', "Div");
    
    // CPP Direct Toke Kinds
    sm_select_base_kind(TokenBaseKind_Comment);
    sm_direct_token_kind("BlockComment");
    sm_direct_token_kind("LineComment");
    
    sm_select_base_kind(TokenBaseKind_Whitespace);
    sm_direct_token_kind("Backslash");
    
    sm_select_base_kind(TokenBaseKind_LiteralInteger);
    sm_direct_token_kind("LiteralInteger");
    sm_direct_token_kind("LiteralIntegerU");
    sm_direct_token_kind("LiteralIntegerL");
    sm_direct_token_kind("LiteralIntegerUL");
    sm_direct_token_kind("LiteralIntegerLL");
    sm_direct_token_kind("LiteralIntegerULL");
    sm_direct_token_kind("LiteralIntegerHex");
    sm_direct_token_kind("LiteralIntegerHexU");
    sm_direct_token_kind("LiteralIntegerHexL");
    sm_direct_token_kind("LiteralIntegerHexUL");
    sm_direct_token_kind("LiteralIntegerHexLL");
    sm_direct_token_kind("LiteralIntegerHexULL");
    sm_direct_token_kind("LiteralIntegerOct");
    sm_direct_token_kind("LiteralIntegerOctU");
    sm_direct_token_kind("LiteralIntegerOctL");
    sm_direct_token_kind("LiteralIntegerOctUL");
    sm_direct_token_kind("LiteralIntegerOctLL");
    sm_direct_token_kind("LiteralIntegerOctULL");
    
    sm_select_base_kind(TokenBaseKind_LiteralFloat);
    sm_direct_token_kind("LiteralFloat32");
    sm_direct_token_kind("LiteralFloat64");
    
    sm_select_base_kind(TokenBaseKind_LiteralString);
    sm_direct_token_kind("LiteralString");
    sm_direct_token_kind("LiteralStringWide");
    sm_direct_token_kind("LiteralStringUTF8");
    sm_direct_token_kind("LiteralStringUTF16");
    sm_direct_token_kind("LiteralStringUTF32");
    sm_direct_token_kind("LiteralStringRaw");
    sm_direct_token_kind("LiteralStringWideRaw");
    sm_direct_token_kind("LiteralStringUTF8Raw");
    sm_direct_token_kind("LiteralStringUTF16Raw");
    sm_direct_token_kind("LiteralStringUTF32Raw");
    sm_direct_token_kind("LiteralCharacter");
    sm_direct_token_kind("LiteralCharacterWide");
    sm_direct_token_kind("LiteralCharacterUTF8");
    sm_direct_token_kind("LiteralCharacterUTF16");
    sm_direct_token_kind("LiteralCharacterUTF32");
    sm_direct_token_kind("PPErrorMessage");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    sm_direct_token_kind("KeywordGeneric");
    
    // CPP Operators
    Operator_Set *main_ops = sm_begin_op_set();
    
    sm_select_base_kind(TokenBaseKind_ScopeOpen);
    sm_op("{");
    sm_select_base_kind(TokenBaseKind_ScopeClose);
    sm_op("}");
    sm_select_base_kind(TokenBaseKind_ParentheticalOpen);
    sm_op("(");
    sm_op("[");
    sm_select_base_kind(TokenBaseKind_ParentheticalClose);
    sm_op(")");
    sm_op("]");
    sm_select_base_kind(TokenBaseKind_StatementClose);
    sm_op(";");
    sm_op(":");
    sm_select_base_kind(TokenBaseKind_Operator);
    sm_op("...");
    
    sm_op("::");
    sm_op("++");
    sm_op("--");
    sm_op(".");
    sm_op("->", "Arrow");
    sm_op("+");
    sm_op("-");
    sm_op("!");
    sm_op("~");
    sm_op("*");
    sm_op("&");
    sm_op(".*");
    sm_op("->*", "ArrowStar");
    sm_op("/");
    sm_op("%");
    
    sm_char_name('<', "Left");
    sm_char_name('>', "Right");
    sm_op("<<");
    sm_op(">>");
    
    sm_op("<=>", "Compare");
    
    sm_char_name('<', "Less");
    sm_char_name('>', "Grtr");
    sm_op("<");
    sm_op("<=");
    sm_op(">");
    sm_op(">=");
    sm_op("==");
    sm_op("!=");
    
    sm_op("^");
    sm_op("|");
    sm_op("&&");
    sm_op("||");
    sm_op("?");
    sm_op("=");
    sm_op("+=");
    sm_op("-=");
    sm_op("*=");
    sm_op("/=");
    sm_op("%=");
    
    sm_char_name('<', "Left");
    sm_char_name('>', "Right");
    sm_op("<<=");
    sm_op(">>=");
    
    sm_select_base_kind(TokenBaseKind_StatementClose);
    sm_op(",");
    
    // CPP Preprocess Operators
    Operator_Set *pp_ops = sm_begin_op_set();
    
    sm_op("#", "PPStringify");
    sm_op("##", "PPConcat");
    
    // CPP Keywords
    Keyword_Set *main_keys = sm_begin_key_set("main_keys");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    sm_key("attribute");
    sm_key("const");
    sm_key("uniform");
    sm_key("varying");
    sm_key("buffer");
    sm_key("shared");
    sm_key("coherent");
    sm_key("volatile");
    sm_key("restrict");
    sm_key("readonly");
    sm_key("writeonly");
    sm_key("atomic_uint");
    sm_key("layout");
    sm_key("centroid");
    sm_key("flat");
    sm_key("smooth");
    sm_key("noperspective");
    sm_key("patch");
    sm_key("sample");
    sm_key("break");
    sm_key("continue");
    sm_key("do");
    sm_key("for");
    sm_key("while");
    sm_key("switch");
    sm_key("case");
    sm_key("default");
    sm_key("if");
    sm_key("else");
    sm_key("subroutine");
    sm_key("in");
    sm_key("out");
    sm_key("inout");
    sm_key("invariant");
    sm_key("discard");
    sm_key("return");
    sm_key("lowp");
    sm_key("mediump");
    sm_key("highp");
    sm_key("precision");
    sm_key("struct");
    sm_key("common");
    sm_key("partition");
    sm_key("active");
    sm_key("asm");
    sm_key("class");
    sm_key("union");
    sm_key("enum");
    sm_key("typedef");
    sm_key("template");
    sm_key("this");
    sm_key("packed");
    sm_key("resource");
    sm_key("goto");
    sm_key("inline");
    sm_key("noinline");
    sm_key("public");
    sm_key("static");
    sm_key("extern");
    sm_key("external");
    sm_key("interface");
    sm_key("superp");
    sm_key("input");
    sm_key("output");
    sm_key("filter");
    sm_key("sizeof");
    sm_key("cast");
    sm_key("namespace");
    sm_key("using");
    sm_key("row_major");
    sm_key("early_fragment_tests");
    
    sm_select_base_kind(TokenBaseKind_Identifier);
    // Built-In types
    sm_key("float");
    sm_key("double");
    sm_key("int");
    sm_key("void");
    sm_key("bool");
    sm_key("mat2");
    sm_key("mat3");
    sm_key("mat4");
    sm_key("dmat2");
    sm_key("dmat3");
    sm_key("dmat4");
    sm_key("mat2x2");
    sm_key("mat2x3");
    sm_key("mat2x4");
    sm_key("dmat2x2");
    sm_key("dmat2x3");
    sm_key("dmat2x4");
    sm_key("mat3x2");
    sm_key("mat3x3");
    sm_key("mat3x4");
    sm_key("dmat3x2");
    sm_key("dmat3x3");
    sm_key("dmat3x4");
    sm_key("mat4x2");
    sm_key("mat4x3");
    sm_key("mat4x4");
    sm_key("dmat4x2");
    sm_key("dmat4x3");
    sm_key("dmat4x4");
    sm_key("vec2");
    sm_key("vec3");
    sm_key("vec4");
    sm_key("ivec2");
    sm_key("ivec3");
    sm_key("ivec4");
    sm_key("bvec2");
    sm_key("bvec3");
    sm_key("bvec4");
    sm_key("dvec2");
    sm_key("dvec3");
    sm_key("dvec4");
    sm_key("uint");
    sm_key("uvec2");
    sm_key("uvec3");
    sm_key("uvec4");
    sm_key("sampler1D");
    sm_key("sampler2D");
    sm_key("sampler3D");
    sm_key("samplerCube");
    sm_key("sampler1DShadow");
    sm_key("sampler2DShadow");
    sm_key("samplerCubeShadow");
    sm_key("sampler1DArray");
    sm_key("sampler2DArray");
    sm_key("sampler1DArrayShadow");
    sm_key("sampler2DArrayShadow");
    sm_key("isampler1D");
    sm_key("isampler2D");
    sm_key("isampler3D");
    sm_key("isamplerCube");
    sm_key("isampler1DArray");
    sm_key("isampler2DArray");
    sm_key("usampler1D");
    sm_key("usampler2D");
    sm_key("usampler3D");
    sm_key("usamplerCube");
    sm_key("usampler1DArray");
    sm_key("usampler2DArray");
    sm_key("sampler2DRect");
    sm_key("sampler2DRectShadow");
    sm_key("isampler2DRect");
    sm_key("usampler2DRect");
    sm_key("samplerBuffer");
    sm_key("isamplerBuffer");
    sm_key("usamplerBuffer");
    sm_key("sampler2DMS");
    sm_key("isampler2DMS");
    sm_key("usampler2DMS");
    sm_key("sampler2DMSArray");
    sm_key("isampler2DMSArray");
    sm_key("usampler2DMSArray");
    sm_key("samplerCubeArray");
    sm_key("samplerCubeArrayShadow");
    sm_key("isamplerCubeArray");
    sm_key("usamplerCubeArray");
    sm_key("image1D");
    sm_key("iimage1D");
    sm_key("uimage1D");
    sm_key("image2D");
    sm_key("iimage2D");
    sm_key("uimage2D");
    sm_key("image3D");
    sm_key("iimage3D");
    sm_key("uimage3D");
    sm_key("image2DRect");
    sm_key("iimage2DRect");
    sm_key("uimage2DRect");
    sm_key("imageCube");
    sm_key("iimageCube");
    sm_key("uimageCube");
    sm_key("imageBuffer");
    sm_key("iimageBuffer");
    sm_key("uimageBuffer");
    sm_key("image1DArray");
    sm_key("iimage1DArray");
    sm_key("uimage1DArray");
    sm_key("image2DArray");
    sm_key("iimage2DArray");
    sm_key("uimage2DArray");
    sm_key("imageCubeArray");
    sm_key("iimageCubeArray");
    sm_key("uimageCubeArray");
    sm_key("image2DMS");
    sm_key("iimage2DMS");
    sm_key("uimage2DMS");
    sm_key("image2DMSArray");
    sm_key("iimage2DMSArray");
    sm_key("uimage2DMSArray");
    sm_key("long");
    sm_key("short");
    sm_key("half");
    sm_key("fixed");
    sm_key("unsigned");
    sm_key("hvec2");
    sm_key("hvec3");
    sm_key("hvec4");
    sm_key("fvec2");
    sm_key("fvec3");
    sm_key("fvec4");
    sm_key("sampler3DRect");
    
    // Built-In Functions
    sm_key("abs");
    sm_key("acos");
    sm_key("acosh");
    sm_key("all");
    sm_key("any");
    sm_key("asin");
    sm_key("asinh");
    sm_key("atan");
    sm_key("atanh");
    sm_key("atomicCounter");
    sm_key("atomicCounterDecrement");
    sm_key("atomicCounterIncrement");
    sm_key("barrier");
    sm_key("bitCount");
    sm_key("bitfieldExtract");
    sm_key("bitfieldInsert");
    sm_key("bitfieldReverse");
    sm_key("ceil");
    sm_key("clamp");
    sm_key("cos");
    sm_key("cosh");
    sm_key("cross");
    sm_key("degrees");
    sm_key("determinant");
    sm_key("dFdx");
    sm_key("dFdy");
    sm_key("dFdyFine");
    sm_key("dFdxFine");
    sm_key("dFdyCoarse");
    sm_key("dFdxCourse");
    sm_key("fwidthFine");
    sm_key("fwidthCoarse");
    sm_key("distance");
    sm_key("dot");
    sm_key("EmitStreamVertex");
    sm_key("EmitVertex");
    sm_key("EndPrimitive");
    sm_key("EndStreamPrimitive");
    sm_key("equal");
    sm_key("exp");
    sm_key("exp2");
    sm_key("faceforward");
    sm_key("findLSB");
    sm_key("findMSB");
    sm_key("floatBitsToInt");
    sm_key("floatBitsToUint");
    sm_key("floor");
    sm_key("fma");
    sm_key("fract");
    sm_key("frexp");
    sm_key("fwidth");
    sm_key("greaterThan");
    sm_key("greaterThanEqual");
    sm_key("imageAtomicAdd");
    sm_key("imageAtomicAnd");
    sm_key("imageAtomicCompSwap");
    sm_key("imageAtomicExchange");
    sm_key("imageAtomicMax");
    sm_key("imageAtomicMin");
    sm_key("imageAtomicOr");
    sm_key("imageAtomicXor");
    sm_key("imageLoad");
    sm_key("imageSize");
    sm_key("imageStore");
    sm_key("imulExtended");
    sm_key("intBitsToFloat");
    sm_key("imageSamples");
    sm_key("interpolateAtCentroid");
    sm_key("interpolateAtOffset");
    sm_key("interpolateAtSample");
    sm_key("inverse");
    sm_key("inversesqrt");
    sm_key("isinf");
    sm_key("isnan");
    sm_key("ldexp");
    sm_key("length");
    sm_key("lessThan");
    sm_key("lessThanEqual");
    sm_key("log");
    sm_key("log2");
    sm_key("matrixCompMult");
    sm_key("max");
    sm_key("memoryBarrier");
    sm_key("min");
    sm_key("mix");
    sm_key("mod");
    sm_key("modf");
    sm_key("noise");
    sm_key("normalize");
    sm_key("not");
    sm_key("notEqual");
    sm_key("outerProduct");
    sm_key("packDouble2x32");
    sm_key("packHalf2x16");
    sm_key("packSnorm2x16");
    sm_key("packSnorm4x8");
    sm_key("packUnorm2x16");
    sm_key("packUnorm4x8");
    sm_key("pow");
    sm_key("radians");
    sm_key("reflect");
    sm_key("refract");
    sm_key("round");
    sm_key("roundEven");
    sm_key("sign");
    sm_key("sin");
    sm_key("sinh");
    sm_key("smoothstep");
    sm_key("sqrt");
    sm_key("step");
    sm_key("tan");
    sm_key("tanh");
    sm_key("texelFetch");
    sm_key("texelFetchOffset");
    sm_key("texture");
    sm_key("textureGather");
    sm_key("textureGatherOffset");
    sm_key("textureGatherOffsets");
    sm_key("textureGrad");
    sm_key("textureGradOffset");
    sm_key("textureLod");
    sm_key("textureLodOffset");
    sm_key("textureOffset");
    sm_key("textureProj");
    sm_key("textureProjGrad");
    sm_key("textureProjGradOffset");
    sm_key("textureProjLod");
    sm_key("textureProjLodOffset");
    sm_key("textureProjOffset");
    sm_key("textureQueryLevels");
    sm_key("textureQueryLod");
    sm_key("textureSize");
    sm_key("transpose");
    sm_key("trunc");
    sm_key("uaddCarry");
    sm_key("uintBitsToFloat");
    sm_key("umulExtended");
    sm_key("unpackDouble2x32");
    sm_key("unpackHalf2x16");
    sm_key("unpackSnorm2x16");
    sm_key("unpackSnorm4x8");
    sm_key("unpackUnorm2x16");
    sm_key("unpackUnorm4x8");
    sm_key("usubBorrow");
    
    // Deprecated
    sm_key("texture1D");
    sm_key("texture1DProj");
    sm_key("texture1DLod");
    sm_key("texture1DProjLod");
    sm_key("texture2D");
    sm_key("texture2DProj");
    sm_key("texture2DLod");
    sm_key("texture2DProjLod");
    sm_key("texture2DRect");
    sm_key("texture2DRectProj");
    sm_key("texture3D");
    sm_key("texture3DProj");
    sm_key("texture3DLod");
    sm_key("texture3DProjLod");
    sm_key("shadow1D");
    sm_key("shadow1DProj");
    sm_key("shadow1DLod");
    sm_key("shadow1DProjLod");
    sm_key("shadow2D");
    sm_key("shadow2DProj");
    sm_key("shadow2DLod");
    sm_key("shadow2DProjLod");
    sm_key("textureCube");
    sm_key("textureCubeLod");
    
    sm_select_base_kind(TokenBaseKind_LiteralInteger);
    sm_key("LiteralTrue", "true");
    sm_key("LiteralFalse", "false");
    
    sm_select_base_kind(TokenBaseKind_Identifier);
    sm_key_fallback("Identifier");
    
    // CPP Preprocess Directives
    Keyword_Set *pp_directive_set = sm_begin_key_set("pp_directives");
    
    sm_select_base_kind(TokenBaseKind_Preprocessor);
    sm_key("PPVersion", "version");
    sm_key("PPDefine", "define");
    sm_key("PPUndef", "undef");
    sm_key("PPIf", "if");
    sm_key("PPIfDef", "ifdef");
    sm_key("PPIfndef", "ifndef");
    sm_key("PPElse", "else");
    sm_key("PPElIf", "elif");
    sm_key("PPEndIf", "endif");
    sm_key("PPError", "error");
    sm_key("PPLine", "line");
    sm_key("PPPragma", "pragma");
    sm_key("PPExtension", "extension");
    
    sm_select_base_kind(TokenBaseKind_LexError);
    sm_key_fallback("PPUnknown");
    
    // CPP Preprocess Keywords
    Keyword_Set *pp_keys = sm_begin_key_set("pp_keys");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    sm_key("PPDefined", "defined");
    
    // State Machine
    State *root = sm_begin_state_machine();
    
    Flag *is_hex = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_oct = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_wide  = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_utf8  = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_utf16 = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_utf32 = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_char  = sm_add_flag(FlagResetRule_AutoZero);
    
    Flag *is_pp_body      = sm_add_flag(FlagResetRule_KeepState);
    Flag *is_error_body   = sm_add_flag(FlagResetRule_KeepState);
    
    sm_flag_bind(is_pp_body, TokenBaseFlag_PreprocessorBody);
    
#define AddState(N) State *N = sm_add_state(#N)
    
    AddState(identifier);
    AddState(whitespace);
    AddState(whitespace_end_pp);
    AddState(error_body);
    AddState(backslash);
    
    AddState(operator_or_fnumber_dot);
    AddState(operator_or_comment_slash);
    
    AddState(number);
    AddState(znumber);
    
    AddState(fnumber_decimal);
    AddState(fnumber_exponent);
    AddState(fnumber_exponent_sign);
    AddState(fnumber_exponent_digits);
    
    AddState(number_hex_first);
    AddState(number_hex);
    AddState(number_oct);
    
    AddState(U_number);
    AddState(L_number);
    AddState(UL_number);
    AddState(LU_number);
    AddState(l_number);
    AddState(Ul_number);
    AddState(lU_number);
    AddState(LL_number);
    AddState(ULL_number);
    
    AddState(pp_directive_whitespace);
    AddState(pp_directive_first);
    AddState(pp_directive);
    AddState(pp_directive_emit);
    
    AddState(pre_L);
    AddState(pre_u);
    AddState(pre_U);
    AddState(pre_u8);
    AddState(pre_R);
    
    AddState(character);
    AddState(string);
    AddState(string_esc);
    AddState(string_esc_oct2);
    AddState(string_esc_oct1);
    AddState(string_esc_hex);
    AddState(string_esc_universal_8);
    AddState(string_esc_universal_7);
    AddState(string_esc_universal_6);
    AddState(string_esc_universal_5);
    AddState(string_esc_universal_4);
    AddState(string_esc_universal_3);
    AddState(string_esc_universal_2);
    AddState(string_esc_universal_1);
    
    AddState(raw_string);
    AddState(raw_string_get_delim);
    AddState(raw_string_finish_delim);
    AddState(raw_string_find_close);
    AddState(raw_string_try_delim);
    AddState(raw_string_try_quote);
    
    AddState(comment_block);
    AddState(comment_block_try_close);
    AddState(comment_block_newline);
    AddState(comment_line);
    
    Operator_Set *main_ops_without_dot_or_slash = smo_copy_op_set(main_ops);
    smo_remove_ops_with_prefix(main_ops_without_dot_or_slash, ".");
    smo_remove_ops_with_prefix(main_ops_without_dot_or_slash, "/");
    
    Operator_Set *main_ops_with_dot = smo_copy_op_set(main_ops);
    smo_remove_ops_without_prefix(main_ops_with_dot, ".");
    smo_ops_string_skip(main_ops_with_dot, 1);
    
    ////
    
    sm_select_state(root);
    
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("EOF");
        sm_case_eof(emit);
    }
    
    sm_case("abcdefghijklmnopqrstvwxyz"
            "ABCDEFGHIJKMNOPQSTVWXYZ"
            "_$",
            identifier);
    sm_case(utf8, identifier);
    sm_case("L", pre_L);
    sm_case("u", pre_u);
    sm_case("U", pre_U);
    sm_case("R", pre_R);
    
    sm_case_flagged(is_error_body, true, " \r\t\f\v", error_body);
    sm_case_flagged(is_error_body, false, " \r\t\f\v", whitespace);
    sm_case("\n", whitespace_end_pp);
    sm_case("\\", backslash);
    
    sm_case(".", operator_or_fnumber_dot);
    sm_case("/", operator_or_comment_slash);
    {
        Character_Set *char_set = smo_new_char_set();
        smo_char_set_union_ops_firsts(char_set, main_ops_without_dot_or_slash);
        smo_char_set_remove(char_set, ".</");
        char *char_set_array = smo_char_set_get_array(char_set);
        State *operator_state = smo_op_set_lexer_root(main_ops_without_dot_or_slash, root, "LexError");
        sm_case_peek(char_set_array, operator_state);
    }
    
    sm_case("123456789", number);
    sm_case("0", znumber);
    
    sm_case("\'", character);
    sm_case_flagged(is_pp_body, false, "#", pp_directive_whitespace);
    {
        State *operator_state = smo_op_set_lexer_root(pp_ops, root, "LexError");
        sm_case_peek_flagged(is_pp_body, true, "#", operator_state);
    }
    
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback(emit);
    }
    
    ////
    
    sm_select_state(identifier);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_$"
            "0123456789",
            identifier);
    sm_case(utf8, identifier);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_keys(is_pp_body, pp_keys);
        sm_emit_handler_keys(main_keys);
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(whitespace);
    sm_case(" \t\r\f\v", whitespace);
    sm_case("\n", whitespace_end_pp);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Whitespace");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(whitespace_end_pp);
    sm_set_flag(is_pp_body, false);
    sm_set_flag(is_error_body, false);
    sm_fallback_peek(whitespace);
    
    ////
    
    sm_select_state(error_body);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("PPErrorMessage");
        sm_case_peek("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("PPErrorMessage");
        sm_case_eof_peek(emit);
    }
    sm_fallback(error_body);
    
    ////
    
    sm_select_state(backslash);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Backslash");
        sm_case("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Backslash");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(operator_or_comment_slash);
    sm_case("*", comment_block);
    sm_case("/", comment_line);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("DivEq");
        sm_case("=", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Div");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(operator_or_fnumber_dot);
    sm_case("0123456789", fnumber_decimal);
    {
        Character_Set *char_set = smo_new_char_set();
        smo_char_set_union_ops_firsts(char_set, main_ops_with_dot);
        char *char_set_array = smo_char_set_get_array(char_set);
        State *operator_state = smo_op_set_lexer_root(main_ops_with_dot, root, "LexError");
        sm_case_peek(char_set_array, operator_state);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("Dot");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number);
    sm_case("0123456789", number);
    sm_case(".", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralInteger");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(znumber);
    sm_case(".", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    sm_case("Xx", number_hex_first);
    sm_case("01234567", number_oct);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralInteger");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(fnumber_decimal);
    sm_case("0123456789", fnumber_decimal);
    sm_case("Ee", fnumber_exponent);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(fnumber_exponent);
    sm_case("+-", fnumber_exponent_sign);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(fnumber_exponent_sign);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(fnumber_exponent_digits);
    sm_case("0123456789", fnumber_exponent_digits);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat32");
        sm_case("Ff", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_case("Ll", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralFloat64");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number_hex_first);
    sm_set_flag(is_hex, true);
    sm_case("0123456789abcdefABCDEF", number_hex);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number_hex);
    sm_case("0123456789abcdefABCDEF", number_hex);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralIntegerHex");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number_oct);
    sm_set_flag(is_oct, true);
    sm_case("01234567", number_oct);
    sm_case("Uu", U_number);
    sm_case("L", L_number);
    sm_case("l", l_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralIntegerOct");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(U_number);
    sm_case("L", UL_number);
    sm_case("l", Ul_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexU");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctU");
        sm_emit_handler_direct("LiteralIntegerU");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(L_number);
    sm_case("L", LL_number);
    sm_case("Uu", LU_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctL");
        sm_emit_handler_direct("LiteralIntegerL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(l_number);
    sm_case("l", LL_number);
    sm_case("Uu", lU_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctL");
        sm_emit_handler_direct("LiteralIntegerL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(LL_number);
    sm_case("Uu", ULL_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexLL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctLL");
        sm_emit_handler_direct("LiteralIntegerLL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(UL_number);
    sm_case("L", ULL_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexUL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctUL");
        sm_emit_handler_direct("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(Ul_number);
    sm_case("l", ULL_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexUL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctUL");
        sm_emit_handler_direct("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(LU_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexUL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctUL");
        sm_emit_handler_direct("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(lU_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexUL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctUL");
        sm_emit_handler_direct("LiteralIntegerUL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(ULL_number);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_hex, "LiteralIntegerHexULL");
        sm_emit_handler_direct(is_oct, "LiteralIntegerOctULL");
        sm_emit_handler_direct("LiteralIntegerULL");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(pp_directive_whitespace);
    sm_case(" \t\f\v", pp_directive_whitespace);
    sm_case_peek("abcdefghijklmnopqrstuvwxyz"
                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                 "_"
                 "0123456789",
                 pp_directive_first);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(pp_directive_first);
    sm_delim_mark_first();
    sm_set_flag(is_pp_body, true);
    sm_fallback_peek(pp_directive);
    
    ////
    
    sm_select_state(pp_directive);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_"
            "0123456789",
            pp_directive);
    sm_fallback_peek(pp_directive_emit);
    
    ////
    
    sm_select_state(pp_directive_emit);
    sm_delim_mark_one_past_last();
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_check_set_flag("PPError", is_error_body, true);
        sm_emit_handler_keys_delim(pp_directive_set);
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(pre_L);
    sm_set_flag(is_wide, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(pre_u);
    sm_set_flag(is_utf16, true);
    sm_case("\"", string);
    sm_case("8", pre_u8);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(pre_U);
    sm_set_flag(is_utf32, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(pre_u8);
    sm_set_flag(is_utf8, true);
    sm_case("\"", string);
    sm_case("R", pre_R);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(pre_R);
    sm_case("\"", raw_string);
    sm_fallback_peek(identifier);
    
    ////
    
    sm_select_state(character);
    sm_set_flag(is_char, true);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_wide, "LiteralStringWide");
        sm_emit_handler_direct(is_utf8 , "LiteralStringUTF8");
        sm_emit_handler_direct(is_utf16, "LiteralStringUTF16");
        sm_emit_handler_direct(is_utf32, "LiteralStringUTF32");
        sm_emit_handler_direct("LiteralString");
        sm_case_flagged(is_char, false, "\"", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_wide, "LiteralCharacterWide");
        sm_emit_handler_direct(is_utf8 , "LiteralCharacterUTF8");
        sm_emit_handler_direct(is_utf16, "LiteralCharacterUTF16");
        sm_emit_handler_direct(is_utf32, "LiteralCharacterUTF32");
        sm_emit_handler_direct("LiteralCharacter");
        sm_case_flagged(is_char, true, "\'", emit);
    }
    sm_case("\\", string_esc);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_peek("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_case_flagged(is_char, true, "\"", string);
    sm_case_flagged(is_char, false, "\'", string);
    sm_fallback(string);
    
    ////
    
    sm_select_state(string_esc);
    sm_case("\n'\"?\\abfnrtv", string);
    sm_case("01234567", string_esc_oct2);
    sm_case("x", string_esc_hex);
    sm_case("u", string_esc_universal_4);
    sm_case("U", string_esc_universal_8);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_peek("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_fallback(string);
    
    ////
    
    sm_select_state(string_esc_oct2);
    sm_case("01234567", string_esc_oct1);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_oct1);
    sm_case("01234567", string);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_hex);
    sm_case("0123456789abcdefABCDEF", string_esc_hex);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_8);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_7);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_7);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_6);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_6);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_5);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_5);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_4);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_4);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_3);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_3);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_2);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_2);
    sm_case("0123456789abcdefABCDEF", string_esc_universal_1);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(string_esc_universal_1);
    sm_case("0123456789abcdefABCDEF", string);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(raw_string);
    sm_delim_mark_first();
    sm_fallback_peek(raw_string_get_delim);
    
    ////
    
    sm_select_state(raw_string_get_delim);
    sm_case_peek("(", raw_string_finish_delim);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case(" \\)", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_fallback(raw_string_get_delim);
    
    ////
    
    sm_select_state(raw_string_finish_delim);
    sm_delim_mark_one_past_last();
    sm_fallback_peek(raw_string_find_close);
    
    ////
    
    sm_select_state(raw_string_find_close);
    sm_case(")", raw_string_try_delim);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_fallback(raw_string_find_close);
    
    ////
    
    sm_select_state(raw_string_try_delim);
    sm_match_delim(raw_string_try_quote, raw_string_find_close);
    
    ////
    
    sm_select_state(raw_string_try_quote);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct(is_wide, "LiteralStringWideRaw");
        sm_emit_handler_direct(is_utf8 , "LiteralStringUTF8Raw");
        sm_emit_handler_direct(is_utf16, "LiteralStringUTF16Raw");
        sm_emit_handler_direct(is_utf32, "LiteralStringUTF32Raw");
        sm_emit_handler_direct("LiteralStringRaw");
        sm_case("\"", emit);
    }
    sm_fallback_peek(raw_string_find_close);
    
    ////
    
    sm_select_state(comment_block);
    sm_case("*", comment_block_try_close);
    sm_case("\n", comment_block_newline);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("BlockComment");
        sm_case_eof_peek(emit);
    }
    sm_fallback(comment_block);
    
    ////
    
    sm_select_state(comment_block_try_close);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("BlockComment");
        sm_case("/", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("BlockComment");
        sm_case_eof_peek(emit);
    }
    sm_case("*", comment_block_try_close);
    sm_fallback(comment_block);
    
    ////
    
    sm_select_state(comment_block_newline);
    sm_set_flag(is_pp_body, false);
    sm_fallback_peek(comment_block);
    
    ////
    
    sm_select_state(comment_line);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LineComment");
        sm_case_peek("\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LineComment");
        sm_case_eof_peek(emit);
    }
    sm_fallback(comment_line);
}

// BOTTOM

