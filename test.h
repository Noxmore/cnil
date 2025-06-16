#ifdef CNIL_TEST
#define DEFINE_TEST(name, ...) \
	void name() __VA_ARGS__
#else
#define CNIL_TEST(name, ...)
#endif