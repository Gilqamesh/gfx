#ifndef TEST_IF_H
# define TEST_IF_H

# if defined(__cplusplus)
#  define PUBLIC_API extern "C"
# else
#  define PUBLIC_API
# endif

PUBLIC_API int next(int a);

#endif // TEST_IF_H
