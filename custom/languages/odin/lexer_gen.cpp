
#define LANG_NAME_LOWER odin
#define LANG_NAME_CAMEL Odin

#include "lexer_generator/4coder_lex_gen_main.cpp"

internal void build_language_model(void)
{
    u8 utf8[129];
    smh_utf8_fill(utf8);
    
    smh_set_base_character_names();
    smh_typical_tokens();
    
    sm_char_name('!', "Not");
    sm_char_name('&', "And");
    sm_char_name('|', "Or");
    sm_char_name('%', "Mod");
    sm_char_name('?', "Ternary");
    sm_char_name('/', "Div");
    
    sm_select_base_kind(TokenBaseKind_Comment);
    sm_direct_token_kind("BlockComment");
    sm_direct_token_kind("LineComment");
    
    sm_select_base_kind(TokenBaseKind_Whitespace);
    sm_direct_token_kind("Backslash");
    
    sm_select_base_kind(TokenBaseKind_LiteralInteger);
    sm_direct_token_kind("LiteralInteger");
    sm_direct_token_kind("LiteralIntegerHex");
    sm_direct_token_kind("LiteralIntegerOct");
    sm_direct_token_kind("LiteralIntegerDoz");
    sm_direct_token_kind("LiteralIntegerBin");
    
    sm_select_base_kind(TokenBaseKind_LiteralFloat);
    sm_direct_token_kind("LiteralFloat");
    
    sm_select_base_kind(TokenBaseKind_LiteralString);
    sm_direct_token_kind("LiteralString");
    sm_direct_token_kind("LiteralCharacter");
    sm_direct_token_kind("LiteralStringRaw");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    sm_direct_token_kind("KeywordGeneric");
    sm_direct_token_kind("KeywordAttribute");
    sm_direct_token_kind("KeywordDirective");
    
    sm_select_base_kind(TokenBaseKind_ParentheticalOpen);
    sm_direct_token_kind("AttributeOpen");
    sm_select_base_kind(TokenBaseKind_ParentheticalClose);
    sm_direct_token_kind("AttributeClose");
    
    // Odin Operators
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
    sm_op("..");
    sm_op("..<");
    sm_op(":");
    
    sm_op("::");
    sm_op(".");
    sm_op("+");
    sm_op("-");
    sm_op("!");
    sm_op("~");
    sm_op("*");
    sm_op("&");
    sm_op("/");
    sm_op("%");
    
    sm_char_name('<', "Left");
    sm_char_name('>', "Right");
    sm_op("<<");
    sm_op(">>");
    
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
    
    
    // Odin Keywords
    Keyword_Set *main_keys = sm_begin_key_set("main_keys");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    
    sm_key("using");
    sm_key("import");
    sm_key("foreign");
    sm_key("package");
    sm_key("where");
    sm_key("when");
    sm_key("if");
    sm_key("else");
    sm_key("for");
    sm_key("switch");
    sm_key("in");
    sm_key("notin");
    sm_key("not_in");
    sm_key("do");
    sm_key("case");
    sm_key("break");
    sm_key("continue");
    sm_key("fallthrough");
    sm_key("defer");
    sm_key("return");
    sm_key("proc");
    sm_key("struct");
    sm_key("union");
    sm_key("enum");
    sm_key("bit_field");
    sm_key("bit_set");
    sm_key("map");
    sm_key("dynamic");
    sm_key("auto_cast");
    sm_key("cast");
    sm_key("transmute");
    sm_key("distinct");
    sm_key("opaque");
    sm_key("no_inline");
    sm_key("size_of");
    sm_key("align_of");
    sm_key("offset_of");
    sm_key("type_of");
    sm_key("context");
    
    sm_select_base_kind(TokenBaseKind_LiteralInteger);
    sm_key("LiteralTrue",  "true");
    sm_key("LiteralFalse", "false");
    sm_key("LiteralNil", "nil");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    
    // Builtin Types
    sm_key("byte");
    sm_key("bool");
    sm_key("b8");
    sm_key("b16");
    sm_key("b32");
    sm_key("b64");
    sm_key("i8");
    sm_key("u8");
    sm_key("i16");
    sm_key("u16");
    sm_key("i32");
    sm_key("u32");
    sm_key("i64");
    sm_key("u64");
    sm_key("i128");
    sm_key("u128");
    sm_key("rune");
    sm_key("f16");
    sm_key("f32");
    sm_key("f64");
    sm_key("complex32");
    sm_key("complex64");
    sm_key("complex128");
    sm_key("quaternion128");
    sm_key("quaternion256");
    sm_key("int");
    sm_key("uint");
    sm_key("uintptr");
    sm_key("rawptr");
    sm_key("string");
    sm_key("cstring");
    sm_key("any");
    sm_key("typeid");
    sm_key("i8le");
    sm_key("u8le");
    sm_key("i16le");
    sm_key("u16le");
    sm_key("i32le");
    sm_key("u32le");
    sm_key("i64le");
    sm_key("u64le");
    sm_key("i128le");
    sm_key("u128le");
    sm_key("i8be");
    sm_key("u8be");
    sm_key("i16be");
    sm_key("u16be");
    sm_key("i32be");
    sm_key("u32be");
    sm_key("i64be");
    sm_key("u64be");
    sm_key("i128be");
    sm_key("u128be");
    
    // Builtin Procs
    sm_key("len");
    sm_key("cap");
    sm_key("size_of");
    sm_key("align_of");
    sm_key("offset_of");
    sm_key("type_of");
    sm_key("type_info_of");
    sm_key("typeid_of");
    sm_key("swizzle");
    sm_key("complex");
    sm_key("quaternion");
    sm_key("real");
    sm_key("imag");
    sm_key("jmag");
    sm_key("kmag");
    sm_key("conj");
    sm_key("expand_to_tuple");
    sm_key("min");
    sm_key("max");
    sm_key("abs");
    sm_key("clamp");
    
    // @builtins
    sm_key("init_global_temporary_allocator");
    sm_key("copy_slice");
    sm_key("copy_from_string");
    sm_key("copy");
    sm_key("pop");
    sm_key("unoredered_remove");
    sm_key("ordered_remove");
    sm_key("clear");
    sm_key("reserve");
    sm_key("resize");
    sm_key("free");
    sm_key("free_all");
    sm_key("delete_string");
    sm_key("delete_cstring");
    sm_key("delete_dynamic_array");
    sm_key("delete_slice");
    sm_key("delete_map");
    sm_key("delete");
    sm_key("new");
    sm_key("new_clone");
    sm_key("make_slice");
    sm_key("make_dynamic_array");
    sm_key("make_dynamic_array_len");
    sm_key("make_dynamic_array_len_cap");
    sm_key("make_map");
    sm_key("make");
    sm_key("reserve_map");
    sm_key("delete_key");
    sm_key("append_elem");
    sm_key("append_elems");
    sm_key("append_elem_string");
    sm_key("reserve_soa");
    sm_key("append_soa_elem");
    sm_key("append_soa_elems");
    sm_key("append");
    sm_key("append_soa");
    sm_key("append_string");
    sm_key("clear_dynamic_array");
    sm_key("reserve_dynamic_array");
    sm_key("resize_dynamic_array");
    sm_key("incl_elem");
    sm_key("incl_elems");
    sm_key("excl_elem");
    sm_key("excl_elems");
    sm_key("excl_bit_set");
    sm_key("incl");
    sm_key("excl");
    sm_key("card");
    
    sm_select_base_kind(TokenBaseKind_Identifier);
    sm_key_fallback("Identifier");
    
    Keyword_Set *directive_set = sm_begin_key_set("directives");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    
    sm_key("align");
    sm_key("packed");
    sm_key("raw_union");
    sm_key("no_nil");
    sm_key("complete");
    sm_key("no_alias");
    sm_key("type");
    sm_key("c_vararg");
    sm_key("assert");
    sm_key("caller_location");
    sm_key("file");
    sm_key("line");
    sm_key("locations");
    sm_key("procedure");
    sm_key("load");
    sm_key("defined");
    sm_key("bounds_check");
    sm_key("no_bounds_check");
    sm_key("partial");
    sm_key("force_inline");
    sm_key("unroll");
    
    Keyword_Set *attribute_set = sm_begin_key_set("attributes");
    
    sm_select_base_kind(TokenBaseKind_Keyword);
    
    sm_key("builtin");
    sm_key("export");
    sm_key("static");
    sm_key("deferred_in");
    sm_key("deferred_none");
    sm_key("deferred_out");
    sm_key("require_results");
    sm_key("default_calling_convention");
    sm_key("link_name");
    sm_key("link_prefix");
    sm_key("deprecated");
    sm_key("private");
    sm_key("thread_local");
    
    // State Machine
    State *root = sm_begin_state_machine();
    
    Flag *is_hex = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_oct = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_doz = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_bin = sm_add_flag(FlagResetRule_AutoZero);
    
    Flag *is_char = sm_add_flag(FlagResetRule_AutoZero);
    Flag *is_raw = sm_add_flag(FlagResetRule_AutoZero);
    
    Flag *is_attribute_block = sm_add_flag(FlagResetRule_KeepState);
    
    Flag *TRUE = sm_add_flag(FlagResetRule_KeepState);
    sm_flag_bind(is_attribute_block, TokenBaseFlag_PreprocessorBody);
    
#define AddState(N) State *N = sm_add_state(#N)
    
    AddState(identifier);
    AddState(whitespace);
    
    AddState(operator_or_fnumber_dot);
    AddState(operator_or_comment_slash);
    
    AddState(number);
    AddState(znumber);
    
    AddState(fnumber_decimal);
    AddState(fnumber_exponent);
    AddState(fnumber_exponent_sign);
    AddState(fnumber_exponent_digits);
    
    AddState(number_hex_first);
    AddState(number_doz_first);
    AddState(number_bin_first);
    
    AddState(number_hex);
    AddState(number_oct);
    AddState(number_doz);
    AddState(number_bin);
    
    AddState(character);
    AddState(string);
    AddState(string_esc);
    AddState(raw_string);
    
    AddState(comment_block);
    AddState(comment_block_try_close);
    AddState(comment_line);
    
    AddState(attribute_first);
    AddState(attribute);
    AddState(attribute_emit);
    AddState(attribute_block);
    AddState(attribute_block_end);
    AddState(directive_first);
    AddState(directive);
    AddState(directive_emit);
    
    Operator_Set *main_ops_without_dot_or_slash = smo_copy_op_set(main_ops);
    smo_remove_ops_with_prefix(main_ops_without_dot_or_slash, ".");
    smo_remove_ops_with_prefix(main_ops_without_dot_or_slash, "/");
    
    Operator_Set *main_ops_with_dot = smo_copy_op_set(main_ops);
    smo_remove_ops_without_prefix(main_ops_with_dot, ".");
    smo_ops_string_skip(main_ops_with_dot, 1);
    
    ////
    
    sm_select_state(root);
    
    sm_set_flag(TRUE, true);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("EOF");
        sm_case_eof(emit);
    }
    
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_$",
            identifier);
    sm_case(utf8, identifier);
    
    sm_case(" \r\n\t\f\v", whitespace);
    
    sm_case_flagged(is_attribute_block, true, ")", attribute_block_end);
    sm_case(".", operator_or_fnumber_dot);
    sm_case("/", operator_or_comment_slash);
    {
        Character_Set *char_set = smo_new_char_set();
        smo_char_set_union_ops_firsts(char_set, main_ops_without_dot_or_slash);
        smo_char_set_remove(char_set, "./");
        char *char_set_array = smo_char_set_get_array(char_set);
        State *operator_state = smo_op_set_lexer_root(main_ops_without_dot_or_slash, root, "LexError");
        sm_case_peek(char_set_array, operator_state);
    }
    
    
    sm_case("123456789", number);
    sm_case("0", znumber);
    
    sm_case("`", raw_string);
    sm_case("\"", string);
    sm_case("\'", character);
    
    sm_case("@", attribute_first);
    sm_case("#", directive_first);
    
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
        sm_emit_handler_keys(is_attribute_block, attribute_set);
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
    sm_case("Zz", number_doz_first);
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
    
    sm_select_state(number_doz_first);
    sm_set_flag(is_doz, true);
    sm_case("0123456789abAB", number_doz);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(number_doz);
    sm_case("0123456789abcdefABCDEF", number_doz);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralIntegerDoz");
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
    
    sm_select_state(attribute_first);
    sm_delim_mark_first();
    sm_case("(", attribute_block);
    sm_fallback_peek(attribute);
    
    ////
    
    sm_select_state(attribute);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_",
            attribute);
    sm_fallback_peek(attribute_emit);
    
    ////
    
    sm_select_state(attribute_emit);
    sm_delim_mark_one_past_last();
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_keys_delim(attribute_set);
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(attribute_block);
    sm_set_flag(is_attribute_block, true);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("AttributeOpen");
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(attribute_block_end);
    sm_set_flag(is_attribute_block, false);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("AttributeClose");
        sm_fallback_peek(emit);
    }
    // sm_fallback(root);
    
    ////
    
    sm_select_state(directive_first);
    sm_delim_mark_first();
    sm_fallback_peek(directive);
    
    ////
    
    sm_select_state(directive);
    sm_case("abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "_",
            directive);
    sm_fallback_peek(directive_emit);
    
    ////
    
    sm_select_state(directive_emit);
    sm_delim_mark_one_past_last();
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_keys_delim(directive_set);
        sm_fallback_peek(emit);
    }
    
    ////
    
    sm_select_state(character);
    sm_set_flag(is_char, true);
    sm_fallback_peek(string);
    
    ////
    
    sm_select_state(raw_string);
    sm_set_flag(is_raw, true);
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
        sm_emit_handler_direct("LiteralStringRaw");
        sm_case_flagged(is_raw, true, "`", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LiteralString");
        sm_case_flagged(is_char, false, "\"", emit);
    }
    sm_case_flagged(is_raw, false, "\\", string_esc);
    
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_peek_flagged(is_raw, false, "\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_case_flagged(is_char, true, "\"", string);
    sm_case_flagged(is_raw, true, "`", string);
    sm_case_flagged(is_raw, false, "'", string);
    sm_fallback(string);
    
    ////
    
    sm_select_state(string_esc);
    sm_case("\n'\"?\\abfnrtv01234567xuU", string);
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_peek_flagged(is_raw, false, "\n", emit);
    }
    {
        Emit_Rule *emit = sm_emit_rule();
        sm_emit_handler_direct("LexError");
        sm_case_eof_peek(emit);
    }
    sm_fallback(string);
    
    ////
    
    sm_select_state(comment_block);
    sm_case("*", comment_block_try_close);
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