#ifndef GOT_MYPREDICT_H
#define GOT_MYPREDICT_H

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE !FALSE
#endif

typedef char gchar;

typedef double gdouble;

typedef int gint;
typedef int gboolean;
typedef float gfloat;
typedef unsigned int guint;

#define N_(s) s
#define _(s) s

#define g_free(p) free(p)
#define g_strdup(s) _strdup(s)
#define g_new(t,n) (t*)malloc(sizeof(t)*n)
#define g_try_new(t,n) (t*)malloc(sizeof(t)*n)

#endif	/* GOT_MYPREDICT_H */

