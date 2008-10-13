#ifdef UNDER_CE
#define strerror(a) "Error"
#define errno 0
#define tmpfile() NULL
#endif