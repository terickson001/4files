
#define LANG_NAME_LOWER nasm
#define LANG_NAME_CAMEL Nasm

#include "lexer_generator/4coder_lex_gen_main.cpp"

internal void build_language_model(void)
{
     u8 utf8[129];
     smh_utf8_fill(utf8);
     
     smh_set_base_character_names();
     smh_typical_tokens();
     
     sm_select_base_kind(TokenBaseKind_Comment);
     sm_direct_token_kind("LineComment");
     
     sm_select_base_kind(TokenBaseKind_Whitespace);
     
     sm_select_base_kind(TokenBaseKind_LiteralInteger);
     sm_direct_token_kind("LiteralInteger");
     sm_direct_token_kind("LiteralIntegerHex");
     sm_direct_token_kind("LiteralIntegerOct");
     sm_direct_token_kind("LiteralIntegerBin");
     
     sm_select_base_kind(TokenBaseKind_LiteralFloat);
     sm_direct_token_kind("LiteralFloat");
     
     sm_select_base_kind(TokenBaseKind_LiteralString);
     sm_direct_token_kind("LiteralString");
     sm_direct_token_kind("LiteralCharacter");
     
     sm_select_base_kind(TokenBaseKind_Keyword);
     sm_direct_token_kind("KeywordGeneric");
     
     Operator_Set *main_ops = sm_begin_op_set();
     
     sm_select_base_kind(TokenBaseKind_Operator);
     sm_op(":");
     
     sm_op("(");
     sm_op(")");
     sm_op("[");
     sm_op("]");
     
     sm_op(".");
     sm_op("+");
     sm_op("-");
     sm_op("~");
     
     // NASM Keywords
     Keyword_Set *main_keys = sm_begin_key_set("main_keys");
     
     sm_select_base_kind(TokenBaseKind_Keyword);
     sm_key("absolute");
     sm_key("bits");
     sm_key("common");
     sm_key("cpu");
     sm_key("debug");
     sm_key("default");
     sm_key("extern");
     sm_key("float");
     sm_key("global");
     sm_key("list");
     sm_key("section");
     sm_key("segment");
     sm_key("warning");
     sm_key("sectalign");
     sm_key("export");
     sm_key("group");
     sm_key("import");
     sm_key("library");
     sm_key("map");
     sm_key("module");
     sm_key("org");
     sm_key("osabi");
     sm_key("safeseh");
     sm_key("uppercase");
     
     sm_key("istruc");
     sm_key("at");
     sm_key("iend");
     sm_key("align");
     sm_key("alignb");
     
     sm_key("struc");
     sm_key("endstruc");
     sm_key("__LINE__");
     sm_key("__FILE__");
     
     sm_key("__NASM_MAJOR__");
     sm_key("__NASM_MINOR__");
     sm_key("__NASM_SUBMINOR__");
     
     sm_key("___NASM_PATCHLEVEL__");
     sm_key("__NASM_VERSION_ID__");
     sm_key("__NASM_VER__");
     
     sm_key("__BITS__");
     sm_key("__OUTPUT_FORMAT__");
     sm_key("__DATE__");
     sm_key("__TIME__");
     sm_key("__DATE_NUM__");
     
     sm_key("__TIME_NUM__");
     sm_key("__UTC_DATE__");
     sm_key("__UTC_TIME__");
     sm_key("__UTC_DATE_NUM__");
     
     sm_key("__UTC_TIME_NUM__");
     sm_key("__POSIX_TIME__");
     sm_key("__PASS__");
     sm_key("SECTALIGN");
     
     // NASM Registers
     sm_select_base_kind(TokenBaseKind_Keyword);
     sm_key("rax");
     sm_key("eax");
     sm_key("ax");
     sm_key("ah");
     sm_key("al");
     
     sm_key("rbx");
     sm_key("ebx");
     sm_key("bx");
     sm_key("bh");
     sm_key("bl");
     
     sm_key("rcx");
     sm_key("ecx");
     sm_key("cx");
     sm_key("ch");
     sm_key("cl");
     
     sm_key("rdx");
     sm_key("edx");
     sm_key("dx");
     sm_key("dh");
     sm_key("dl");
     
     sm_key("rsi");
     sm_key("esi");
     sm_key("si");
     sm_key("sil");
     
     sm_key("rdi");
     sm_key("edi");
     sm_key("di");
     sm_key("dil");
     
     sm_key("rsp");
     sm_key("esp");
     sm_key("sp");
     sm_key("spl");
     
     sm_key("rbp");
     sm_key("ebp");
     sm_key("bp");
     sm_key("bpl");
     
     sm_key("r8");
     sm_key("r8d");
     sm_key("r8w");
     sm_key("r8b");
     
     sm_key("r9");
     sm_key("r9d");
     sm_key("r9w");
     sm_key("r9b");
     
     sm_key("r10");
     sm_key("r10d");
     sm_key("r10w");
     sm_key("r10b");
     
     sm_key("r11");
     sm_key("r11d");
     sm_key("r11w");
     sm_key("r11b");
     
     sm_key("r12");
     sm_key("r12d");
     sm_key("r12w");
     sm_key("r12b");
     
     sm_key("r13");
     sm_key("r13d");
     sm_key("r13w");
     sm_key("r13b");
     
     sm_key("r14");
     sm_key("r14d");
     sm_key("r14w");
     sm_key("r14b");
     
     sm_key("r15");
     sm_key("r15d");
     sm_key("r15w");
     sm_key("r15b");
     
     sm_key("rip");
     sm_key("eip");
     sm_key("ip");
     sm_key("ipl");
     
     sm_key("ds");
     sm_key("cs");
     sm_key("ss");
     sm_key("es");
     
     sm_key("rflags");
     sm_key("eflags");
     sm_key("flags");
     
     sm_key("ymm0");
     sm_key("ymm0h");
     sm_key("ymm1");
     sm_key("ymm1h");
     sm_key("ymm2");
     sm_key("ymm2h");
     sm_key("ymm3");
     sm_key("ymm3h");
     sm_key("ymm4");
     sm_key("ymm4h");
     sm_key("ymm5");
     sm_key("ymm5h");
     sm_key("ymm6");
     sm_key("ymm6h");
     sm_key("ymm7");
     sm_key("ymm7h");
     sm_key("ymm8");
     sm_key("ymm8h");
     sm_key("ymm9");
     sm_key("ymm9h");
     sm_key("ymm10");
     sm_key("ymm10h");
     sm_key("ymm11");
     sm_key("ymm11h");
     sm_key("ymm12");
     sm_key("ymm12h");
     sm_key("ymm13");
     sm_key("ymm13h");
     sm_key("ymm14");
     sm_key("ymm14h");
     sm_key("ymm15");
     sm_key("ymm15h");
     
     sm_key("xmm0");
     sm_key("xmm1");
     sm_key("xmm2");
     sm_key("xmm3");
     sm_key("xmm4");
     sm_key("xmm5");
     sm_key("xmm6");
     sm_key("xmm7");
     sm_key("xmm8");
     sm_key("xmm9");
     sm_key("xmm10");
     sm_key("xmm11");
     sm_key("xmm12");
     sm_key("xmm13");
     sm_key("xmm14");
     sm_key("xmm15");
     
     sm_select_base_kind(TokenBaseKind_Identifier);
     sm_key_fallback("Identifier");
     
     // NASM Preprocessor directives
     Keyword_Set *pp_directive_set = sm_begin_key_set("pp_directives");
     
     sm_select_base_kind(TokenBaseKind_Preprocessor);
     sm_key("elif");
     sm_key("elifn");
     sm_key("elifctx");
     sm_key("elifnctx");
     sm_key("elifdef");
     sm_key("elifndef");
     
     sm_key("elifempty");
     sm_key("elifnempty");
     sm_key("elifenv");
     sm_key("elifnenv");
     sm_key("elifid");
     
     sm_key("elifnid");
     sm_key("elifidn");
     sm_key("elifnidn");
     sm_key("elifidni");
     sm_key("elifnidni");
     
     sm_key("elifmacro");
     sm_key("elifnmacro");
     sm_key("elifnum");
     sm_key("elifnnum");
     sm_key("elifstr");
     
     sm_key("elifnstr");
     sm_key("eliftoken");
     sm_key("elifntoken");
     sm_key("if");
     sm_key("ifn");
     sm_key("ifctx");
     
     sm_key("ifnctx");
     sm_key("ifdef");
     sm_key("ifndef");
     sm_key("ifempty");
     sm_key("ifnempty");
     sm_key("ifenv");
     
     sm_key("ifnenv");
     sm_key("ifid");
     sm_key("ifnid");
     sm_key("ifidn");
     sm_key("ifnidn");
     sm_key("ifidni");
     sm_key("ifnidni");
     
     sm_key("ifmacro");
     sm_key("ifnmacro");
     sm_key("ifnum");
     sm_key("ifnnum");
     sm_key("ifstr");
     sm_key("ifnstr");
     
     sm_key("iftoken");
     sm_key("ifntoken");
     sm_key("arg");
     sm_key("assign");
     sm_key("clear");
     sm_key("define");
     
     sm_key("defstr");
     sm_key("deftok");
     sm_key("depend");
     sm_key("else");
     sm_key("endif");
     sm_key("endm");
     sm_key("endmacro");
     
     sm_key("endrep");
     sm_key("error");
     sm_key("exitmacro");
     sm_key("exitrep");
     sm_key("fatal");
     sm_key("iassign");
     
     sm_key("idefine");
     sm_key("idefstr");
     sm_key("ideftok");
     sm_key("imacro");
     sm_key("include");
     sm_key("irmacro");
     
     sm_key("ixdefine");
     sm_key("line");
     sm_key("local");
     sm_key("macro");
     sm_key("pathsearch");
     sm_key("pop");
     sm_key("push");
     
     sm_key("rep");
     sm_key("repl");
     sm_key("rmacro");
     sm_key("rotate");
     sm_key("stacksize");
     sm_key("strcat");
     
     sm_key("strlen");
     sm_key("substr");
     sm_key("undef");
     sm_key("unimacro");
     sm_key("unmacro");
     sm_key("use");
     
     sm_key("warning");
     sm_key("xdefine");
     sm_key("comment");
     sm_key("endcomment");
     
     // State Machine
     State *root = sm_begin_state_machine();
     
     Flag *is_hex = sm_add_flag(FlagResetRule_AutoZero);
     Flag *is_oct = sm_add_flag(FlagResetRule_AutoZero);
     Flag *is_bin = sm_add_flag(FlagResetRule_AutoZero);
     
     Flag *is_char = sm_add_flag(FlagResetRule_AutoZero);
     
     #define AddState(N) State *N = sm_add_state(#N)
     
     AddState(identifier);
     AddState(whitespace);
     
     AddState(operator_or_fnumber_dot);
     
     AddState(number);
     AddState(znumber);
     
     AddState(fnumber_decimal);
     AddState(fnumber_exponent);
     AddState(fnumber_exponent_sign);
     AddState(fnumber_exponent_digits);
     
     AddState(number_hex_first);
     AddState(number_bin_first);
     
     AddState(number_hex);
     AddState(number_oct);
     AddState(number_bin);
     
     AddState(character);
     AddState(string);
     AddState(string_esc);
     
     AddState(comment_line);
     
     AddState(preprocessor_first);
     AddState(preprocessor_rest);
     AddState(preprocessor_emit);
     
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
     sm_case("%", preprocessor_first);
     
     sm_case("abcdefghijklmnopqrstuvwxyz"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "_",
     identifier);
     sm_case(utf8, identifier);
     
     sm_case(" \r\n\t\f\v", whitespace);
     
     sm_case(".", operator_or_fnumber_dot);
     {
         Character_Set *char_set = smo_new_char_set();
         smo_char_set_union_ops_firsts(char_set, main_ops_without_dot_or_slash);
         smo_char_set_remove(char_set, ".");
         char *char_set_array = smo_char_set_get_array(char_set);
         State *operator_state = smo_op_set_lexer_root(main_ops_without_dot_or_slash, root, "LexError");
         sm_case_peek(char_set_array, operator_state);
     }
     
     sm_case("123456789", number);
     sm_case("0", znumber);
     
     sm_case("\"", string);
     sm_case("\'", character);
     
     sm_case(";", comment_line);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LexError");
         sm_fallback(emit);
     }
     
     ////
     
     sm_select_state(identifier);
     sm_case("abcdefghijklmnopqrstuvwxyz"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "_"
             "0123456789",
     identifier);
     sm_case(utf8, identifier);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_keys(main_keys);
         sm_fallback_peek(emit);
     }
     ////
     
     sm_select_state(whitespace);
     sm_case(" \t\r\n\f\v", whitespace);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("Whitespace");
         sm_fallback_peek(emit);
     }
     
     ////
     
     sm_select_state(operator_or_fnumber_dot);
     sm_case("0123456789", fnumber_decimal);
     sm_case_peek("abcdefghijklmnopqrstuvwxyz"
                  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                  "_",
     identifier);
     sm_case_peek(utf8, identifier);
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
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralInteger");
         sm_fallback_peek(emit);
     }
     
     ////
     
     sm_select_state(znumber);
     sm_case(".", fnumber_decimal);
     sm_case("Ee", fnumber_exponent);
     sm_case("Xx", number_hex_first);
     sm_case("Bb", number_bin_first);
     sm_case("Oo", number_oct);
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
         sm_emit_handler_direct("LiteralFloat");
         sm_fallback_peek(emit);
     }
     
     ////
     
     sm_select_state(fnumber_exponent);
     sm_case("+-", fnumber_exponent_sign);
     sm_case("0123456789", fnumber_exponent_digits);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralFloat");
         sm_fallback_peek(emit);
     }
     
     ////
     
     sm_select_state(fnumber_exponent_sign);
     sm_case("0123456789", fnumber_exponent_digits);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralFloat");
         sm_fallback_peek(emit);
     }
     
     ////
     
     sm_select_state(fnumber_exponent_digits);
     sm_case("0123456789", fnumber_exponent_digits);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralFloat");
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
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralIntegerHex");
         sm_fallback_peek(emit);
     }
     
     ////
     
     sm_select_state(number_oct);
     sm_set_flag(is_oct, true);
     sm_case("01234567", number_oct);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralIntegerOct");
         sm_fallback_peek(emit);
     }
     
     ////
     
     sm_select_state(number_bin_first);
     sm_set_flag(is_bin, true);
     sm_case("_", number_bin_first);
     sm_case("01", number_bin);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LexError");
         sm_fallback_peek(emit);
     }
     
     ////
     
     sm_select_state(number_bin);
     sm_case("01_", number_bin);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralIntegerBin");
         sm_fallback_peek(emit);
     }
     
     ////
     
     
     sm_select_state(preprocessor_first);
     sm_delim_mark_first();
     sm_fallback_peek(preprocessor_rest);
     
     ////
     
     sm_select_state(preprocessor_rest);
     sm_case("abcdefghijklmnopqrstuvwxyz"
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "0123456789",
     preprocessor_rest);
     sm_fallback_peek(preprocessor_emit);
     
     ////
     
     sm_select_state(preprocessor_emit);
     sm_delim_mark_one_past_last();
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_keys_delim(pp_directive_set);
         sm_fallback_peek(emit);
     }
     
     
     ///
     
     sm_select_state(character);
     sm_set_flag(is_char, true);
     sm_fallback_peek(string);
     
     ////
     
     sm_select_state(string);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralCharacter");
         sm_case_flagged(is_char, true, "\'", emit);
     }
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LiteralString");
         sm_case_flagged(is_char, false, "\"", emit);
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
     sm_case("\n'\"?\\abfnrtv01234567xuU", string);
     {
         Emit_Rule *emit = sm_emit_rule();
         sm_emit_handler_direct("LexError");
         sm_case_eof_peek(emit);
     }
     sm_fallback(string);
     
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