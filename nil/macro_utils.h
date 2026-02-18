#pragma once

#define NIL_STRINGIFY_INNER(...) #__VA_ARGS__
#define NIL_STRINGIFY(...) NIL_STRINGIFY_INNER(__VA_ARGS__)

#define NIL_UNWRAP_PARENS($macro, $args) $macro $args

#define NIL_IDENTITY(...) __VA_ARGS__

#define NIL_CONCAT_2_INNER(a, b) a##b
#define NIL_CONCAT_2(a, b) NIL_CONCAT_2_INNER(a, b)

#define NIL_GENERATED_COUNTER_NAME(NAME) NIL_GENERATED_COUNTER_NAME_INNER_A(NAME, __COUNTER__)
#define NIL_GENERATED_COUNTER_NAME_INNER_A(NAME, COUNTER) NIL_GENERATED_COUNTER_NAME_INNER_B(NAME, COUNTER)
#define NIL_GENERATED_COUNTER_NAME_INNER_B(NAME, COUNTER) generated__##NAME##COUNTER##__

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

// Like NIL_WRAP_VA_ARGS, but assumes that each arg will be in parentheses, allowing some nicer APIs.
#define NIL_WRAP_VA_ARGS_PARENS($wrapper, ...) NIL_WRAP_VA_ARGS_1($wrapper, __VA_ARGS__)
#define NIL_WRAP_VA_ARGS_PARENS_1($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_2($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_2($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_3($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_3($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_4($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_4($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_5($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_5($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_6($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_6($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_7($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_7($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_8($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_8($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_9($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_9($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_10($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_10($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_11($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_11($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_12($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_12($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_13($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_13($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_14($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_14($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_15($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_15($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_16($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_16($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_17($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_17($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_18($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_18($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_19($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_19($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_20($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_20($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_21($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_21($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_22($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_22($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_23($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_23($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_24($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_24($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_25($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_25($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_26($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_26($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_27($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_27($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_28($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_28($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_29($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_29($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_30($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_30($wrapper, $x, ...) $wrapper $x , __VA_OPT__(NIL_WRAP_VA_ARGS_END($wrapper, __VA_ARGS__))
#define NIL_WRAP_VA_ARGS_PARENS_END($wrapper, $x, ...) too_many_args_max_30

#define NIL_COUNT_ARGS_IMPL(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, _15, _16, _17, _18, _19, _20, _21, _22, _23, _24, _25, _26, _27, _28, _29, _30, N, ...) N
// ## deletes preceding comma if _VA_ARGS__ is empty (GCC, Clang)

// Counts __VA_ARGS__ up to 30 arguments.
#define NIL_COUNT_ARGS(...) NIL_COUNT_ARGS_IMPL(_, ## __VA_ARGS__, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

// Counts any number of __VA_ARGS__, but all supplied arguments must evaluate to ints.
#define NIL_COUNT_ARGS_INTS(...) (sizeof((int[]){ __VA_ARGS__ })/sizeof(int))
// Returns the number of elements in an array. Only works for arrays declared in-scope, not pointer decompositions.
#define NIL_ARRAY_LEN(ARR) (sizeof(ARR)/sizeof(ARR[0]))


// Call FN(INDEX) N times. INDEX starts at 0.
#define NIL_REPEAT(FN, N) NIL_REPEAT_INTERNAL(FN, N)
#define NIL_REPEAT_INTERNAL(FN, N) NIL_REPEAT_##N(FN) // Parameter eval buffer
#define NIL_REPEAT_1(FN) FN(0)
#define NIL_REPEAT_2(FN) NIL_REPEAT_1(FN) FN(1)
#define NIL_REPEAT_3(FN) NIL_REPEAT_2(FN) FN(2)
#define NIL_REPEAT_4(FN) NIL_REPEAT_3(FN) FN(3)
#define NIL_REPEAT_5(FN) NIL_REPEAT_4(FN) FN(4)
#define NIL_REPEAT_6(FN) NIL_REPEAT_5(FN) FN(5)
#define NIL_REPEAT_7(FN) NIL_REPEAT_6(FN) FN(6)
#define NIL_REPEAT_8(FN) NIL_REPEAT_7(FN) FN(7)
#define NIL_REPEAT_9(FN) NIL_REPEAT_8(FN) FN(8)
#define NIL_REPEAT_10(FN) NIL_REPEAT_9(FN) FN(9)
#define NIL_REPEAT_11(FN) NIL_REPEAT_10(FN) FN(10)
#define NIL_REPEAT_12(FN) NIL_REPEAT_11(FN) FN(11)
#define NIL_REPEAT_13(FN) NIL_REPEAT_12(FN) FN(12)
#define NIL_REPEAT_14(FN) NIL_REPEAT_13(FN) FN(13)
#define NIL_REPEAT_15(FN) NIL_REPEAT_14(FN) FN(14)
#define NIL_REPEAT_16(FN) NIL_REPEAT_15(FN) FN(15)
#define NIL_REPEAT_17(FN) NIL_REPEAT_16(FN) FN(16)
#define NIL_REPEAT_18(FN) NIL_REPEAT_17(FN) FN(17)
#define NIL_REPEAT_19(FN) NIL_REPEAT_18(FN) FN(18)
#define NIL_REPEAT_20(FN) NIL_REPEAT_19(FN) FN(19)
#define NIL_REPEAT_21(FN) NIL_REPEAT_20(FN) FN(20)
#define NIL_REPEAT_22(FN) NIL_REPEAT_21(FN) FN(21)
#define NIL_REPEAT_23(FN) NIL_REPEAT_22(FN) FN(22)
#define NIL_REPEAT_24(FN) NIL_REPEAT_23(FN) FN(23)
#define NIL_REPEAT_25(FN) NIL_REPEAT_24(FN) FN(24)
#define NIL_REPEAT_26(FN) NIL_REPEAT_25(FN) FN(25)
#define NIL_REPEAT_27(FN) NIL_REPEAT_26(FN) FN(26)
#define NIL_REPEAT_28(FN) NIL_REPEAT_27(FN) FN(27)
#define NIL_REPEAT_29(FN) NIL_REPEAT_28(FN) FN(28)
#define NIL_REPEAT_30(FN) NIL_REPEAT_29(FN) FN(29)
#define NIL_REPEAT_31(FN) NIL_REPEAT_30(FN) FN(30)

#define NIL_MACRO_VAR($name) NIL_CONCAT_2($name, __LINE__)

#define SCOPED(START, END) \
	for (bool NIL_MACRO_VAR(i_) = (START, false); !NIL_MACRO_VAR(i_); (NIL_MACRO_VAR(i_) = true), END)

#define DEFER(END) \
	for (bool NIL_MACRO_VAR(i_) = false; !NIL_MACRO_VAR(i_); (NIL_MACRO_VAR(i_) = true), END)

// Iterates through an iterator that iterates over pointers. Use like `FOREACH (thing, thing_iter(&things), thing_next) { }
#define FOREACH(VARNAME, ITERATOR, NEXT_FN) \
	auto NIL_MACRO_VAR(iter) = (ITERATOR); \
	for (auto VARNAME = NEXT_FN(&NIL_MACRO_VAR(iter)); VARNAME; VARNAME = NEXT_FN(&NIL_MACRO_VAR(iter)))

#ifdef NIL_INCLUDE_ANNOTATIONS
	#define ANNOTATE(STRING) __attribute__((annotate(STRING)))
#else
	#define ANNOTATE(STRING)
#endif