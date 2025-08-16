#pragma once

#define NIL_STRINGIFY(...) #__VA_ARGS__

#define NIL_UNWRAP_PARENS($macro, $args) $macro $args
// #define NIL_REMOVE_COMMAS(...) NIL_CONCAT_2(NIL_REMOVE_COMMAS_, NIL_COUNT_ARGS(__VA_ARGS__))(__VA_ARGS__)

#define NIL_IDENTITY(...) __VA_ARGS__

// #define EVAL(x) x

#define NIL_CONCAT_2_INNER(a, b) a##b
#define NIL_CONCAT_2(a, b) NIL_CONCAT_2_INNER(a, b)

// #define NIL_EVAL(v) v

#define NIL_WRAP_VA_ARGS($wrapper, ...) NIL_WRAP_VA_ARGS_1($wrapper, __VA_ARGS__)
#define NIL_WRAP_VA_ARGS_1($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_2($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_2($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_3($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_3($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_4($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_4($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_5($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_5($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_6($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_6($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_7($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_7($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_8($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_8($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_9($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_9($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_10($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_10($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_11($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_11($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_12($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_12($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_13($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_13($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_14($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_14($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_15($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_15($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_16($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_16($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_17($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_17($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_18($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_18($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_19($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_19($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_20($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_20($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_21($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_21($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_22($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_22($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_23($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_23($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_24($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_24($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_25($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_25($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_26($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_26($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_27($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_27($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_28($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_28($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_29($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_29($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_30($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_30($wrapper, $x, ...) $wrapper($x), __VA_OPT__(NIL_WRAP_VA_ARGS_END($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_END($wrapper, $x, ...) too_many_args_max_30

#define NIL_COUNT_ARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, _31, N, ...) N
// ## deletes preceding comma if _VA_ARGS__ is empty (GCC, Clang)

// Counts __VA_ARGS__ up to 30 arguments.
#define NIL_COUNT_ARGS(...) NIL_COUNT_ARGS_IMPL(_, ## __VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Counts any number of __VA_ARGS__, but all supplied arguments must evaluate to ints.
#define NIL_COUNT_ARGS_INTS(...) (sizeof((int[]){ __VA_ARGS__ })/sizeof(int))

/*#define FOO_(T) T
#define FOO_(T) T
// #define FOO_const NIL_CONCAT_2(const_, NIL_IDENTITY(
#define FOO_const NIL_CONCAT_2(const_,
// #define FOO_NAMED(T)
// #define FOO_const const_
#define NIL_TYPE_TO_IDENT(T) FOO_##T)*/


/*#define NIL_DECOMPOSED_TYPE_1(T) T
#define NIL_DECOMPOSED_TYPE_2(T, $ptr) T*
#define NIL_DECOMPOSED_TYPE_3(T, $ptr) T*
#define NIL_DECOMPOSED_TYPE(...)*/

/*inline void macro_testing() {
	NIL_REMOVE_COMMAS(const, char, *) bap = "bap!";
	// const char* const_char_ptr = "bap!";
}*/

#define NIL_MACRO_VAR($name) NIL_CONCAT_2($name, __LINE__)

#define defer(start, end) \
	for (bool NIL_MACRO_VAR(i_) = (start, false); !NIL_MACRO_VAR(i_); (NIL_MACRO_VAR(i_) = true), end)

#define scope(end) \
	for (bool NIL_MACRO_VAR(i_) = false; !NIL_MACRO_VAR(i_); (NIL_MACRO_VAR(i_) = true), end)

