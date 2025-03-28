#ifdef NOOST_TEST
#define DEFINE_TEST(name, ...) \
	void name() __VA_ARGS__
#else
#define DEFINE_TEST(name, ...)
#endif