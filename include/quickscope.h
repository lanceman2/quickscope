#ifndef __QUICKSCOPE_H__
#define __QUICKSCOPE_H__

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <gtk/gtk.h>


/**
\c QS_MAJOR is the major version number.
\c QS_MAJOR may be increased for a given release,
but not often. */
#define QS_MAJOR  0
/**
\c QS_MINOR is the minor version number.
\c QS_MINOR may be changed for a given release,
but not often. */
#define QS_MINOR  0
/**
\c QS_EDIT is the edit version number.
\c QS_EDIT should be changed for each release. */
#define QS_EDIT   1

// doxygen skips QS_STR and QS_XSTR
#define QS_STR(s) QS_XSTR(s)
#define QS_XSTR(s) #s

/**
\c QS_VERSION is the version of this quickscope software project
as we define it from the \c QS_MAJOR, \c QS_MINOR, and \c QS_EDIT.
*/
#define QS_VERSION  (QS_STR(QS_MAJOR) "." \
        QS_STR(QS_MINOR) "." QS_STR(QS_EDIT))

// This file may get installed in the "system" (or where ever installer
// decides) so we do not pollute the CPP (C pre-processor) namespace by
// defining EXPORT, instead we define QS_EXPORT
#ifndef QS_EXPORT
#  define QS_EXPORT extern
#endif


#ifdef __cplusplus
extern "C" {
#endif

struct QsApp;
struct QsWindow;

QS_EXPORT const char *qsResourceDir;

QS_EXPORT void qsApp_create(int argc, char **argv, GtkApplication *a);
QS_EXPORT void qsApp_destroy();
QS_EXPORT int qsApp_run(int argc, char **argv);


QS_EXPORT struct QsWindow *qsWindow_create(void);
QS_EXPORT void qsWindow_destroy(struct QsWindow *window);


#ifdef __cplusplus
}
#endif


#endif //#ifndef __QUICKSCOPE_H__

