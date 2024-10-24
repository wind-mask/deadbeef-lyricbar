#pragma once
#ifndef LYRICBAR_MAIN_H
#define LYRICBAR_MAIN_H
#include <gtk/gtk.h>

#include <deadbeef/deadbeef.h>
#include <deadbeef/gtkui_api.h>

#ifdef _WIN32
#ifdef DDB_LYRICS_EXPORT_LIB
#define DDB_LYRICS_EXPORT __declspec(dllexport)
#else
#define DDB_LYRICS_EXPORT __declspec(dllimport)
#endif

#else
#ifdef DDB_LYRICS_EXPORT_LIB
#define DDB_LYRICS_EXPORT __attribute__((visibility("default")))
#else
#define DDB_LYRICS_EXPORT
#endif
#endif

extern int death_signal;
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  ddb_gtkui_widget_t base;
} widget_lyricbar_t;

extern DDB_LYRICS_EXPORT DB_functions_t *deadbeef;

#ifdef __cplusplus
}
#endif

#endif // LYRICBAR_MAIN_H
