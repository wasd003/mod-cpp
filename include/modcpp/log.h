#include <cstdio>

#ifdef DEBUG
#define log(fmt, args...) \
    do { \
        printf("[%s:%d] " fmt, __func__, __LINE__, ##args); \
    } while (0)
#else
#define log(fmt, args...) do {} while (0)
#endif

#define trace(...) printf("------ %s ------\n", __func__);
