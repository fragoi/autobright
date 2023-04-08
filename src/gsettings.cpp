#include <string>

#include "gsettings.h"
#include "gexception.h"

using namespace std;

namespace gsettings {

  static const char *SCHEMA_ID = "fragoi.autobright";

  PGSettings newDefault() {
    return PGSettings(g_settings_new(SCHEMA_ID));
  }

  PGSettings newFromPath(const char *path) {
    string schemaPath(path);
    schemaPath.append("/schemas/");

    GException error;

    gboxed_ptr<GSettingsSchemaSource> source(
        g_settings_schema_source_new_from_directory(
            schemaPath.c_str(),
            g_settings_schema_source_get_default(),
            FALSE,
            error.get()),
        g_settings_schema_source_unref);

    if (error)
      throw error;

    gboxed_ptr<GSettingsSchema> schema(
        g_settings_schema_source_lookup(
            source.get(),
            SCHEMA_ID,
            FALSE),
        g_settings_schema_unref);

    PGSettings settings(
        g_settings_new_full(schema.get(), NULL, NULL));

    return settings;
  }

}
