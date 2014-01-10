/**
 * Copyright (c) 2013 Genome Research Ltd. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @file baton.h
 * @author Keith James <kdj@sanger.ac.uk>
 */

#ifndef _BATON_H
#define _BATON_H

#include "rcConnect.h"
#include "rodsClient.h"
#include "rodsPath.h"
#include <jansson.h>

#include "utilities.h"

#define ACCESS_READ       "read"
#define ACCESS_WRITE      "write"
#define ACCESS_NAMESPACE  "access_type"

#define MAX_NUM_CONDITIONALS 20
#define MAX_ERROR_MESSAGE_LEN 1024

#define META_ADD_NAME "add"
#define META_REM_NAME "rm"

#define SEARCH_OP_EQUALS "="
#define SEARCH_OP_LIKE   "like"

#define JSON_ATTRIBUTE_KEY  "attribute"
#define JSON_VALUE_KEY      "value"
#define JSON_UNITS_KEY      "units"
#define JSON_AVUS_KEY       "avus"
#define JSON_OPERATOR_KEY   "operator"
#define JSON_ACCESS_KEY     "access"
#define JSON_OWNER_KEY      "owner"
#define JSON_LEVEL_KEY      "level"


/**
 *  @enum metadata_op
 *  @brief AVU metadata operations.
 */
typedef enum {
    /** Add an AVU. */
    META_ADD,
    /** Remove an AVU. */
    META_REM,
    /** Query AVUs */
    META_QUERY
} metadata_op;

typedef enum {
    /** Non-recursive operation */
    NO_RECURSE,
    /** Recursive operation */
    RECURSE
} recursive_op;

/**
 *  @struct metadata_op
 *  @brief AVU metadata operation inputs.
 */
struct mod_metadata_in {
    /** The operation to perform. */
    metadata_op op;
    /** The type argument for the iRODS path i.e. -d or -C. */
    char *type_arg;
    /** The resolved iRODS path. */
    rodsPath_t *rods_path;
    /** The AVU attribute name. */
    char *attr_name;
    /** The AVU attribute value. */
    char *attr_value;
    /** The AVU attribute units. */
    char *attr_units;
};

typedef struct query_cond {
    /** The ICAT column to match e.g. COL_META_DATA_ATTR_NAME */
    int column;
    /** The operator to use e.g. "=", "<", ">" */
    const char *operator;
    /** The value to match */
    const char *value;
} query_cond_t;

typedef struct baton_error {
    /** Error code */
    int code;
    /** Error message */
    char message[MAX_ERROR_MESSAGE_LEN];
    /** Error message length */
    size_t size;
} baton_error_t;


/**
 * Log the current iRODS error stack through the underlying logging
 * mechanism.
 *
 * @param[in] level     The logging level.
 * @param[in] category  The log message category.
 * @param[in] error     The iRODS error state.
 */
void log_rods_errstack(log_level level, const char *category, rError_t *error);

/**
 * Log the current JSON error state through the underlying logging
 * mechanism.
 *
 * @param[in] level     The logging level.
 * @param[in] category  The log message category.
 * @param[in] error     The JSON error state.
 */
void log_json_error(log_level level, const char *category,
                    json_error_t *error);

void init_baton_error(baton_error_t *error);

/**
 * Set error state information. The size field will be set to the
 * length of the formatted message.
 *
 * @param[in] error      The error struct to modify.
 * @param[in] code       The error code.
 * @param[in] format     The error message format string or template.
 * @param[in] arguments  The format arguments.
 */
void set_baton_error(baton_error_t *error, int code,
                     const char *format, ...);

/**
 * Test that a connection can be made to the server.
 *
 * @return 1 on success, 0 on failure.
 */
int is_irods_available();

/**
 * Log into iRODS using an pre-defined environment.
 *
 * @param[in] env A populated iRODS environment.
 *
 * @return An open connection to the iRODS server or NULL on error.
 */
rcComm_t *rods_login(rodsEnv *env);

/**
 * Initialise an iRODS path by copying a string into its inPath.
 *
 * @param[out] rods_path An iRODS path.
 * @param[in]  inpath    A string representing an unresolved iRODS path.
 *
 * @return 0 on success, -1 on failure.
 */
int init_rods_path(rodsPath_t *rods_path, char *inpath);

/**
 * Initialise and resolve an iRODS path by copying a string into its
 * inPath, parsing it and resolving it on the server.
 *
 * @param[in]  conn      An open iRODS connection.
 * @param[in]  env       A populated iRODS environment.
 * @param[out] rodspath  An iRODS path.
 * @param[in]  inpath    A string representing an unresolved iRODS path.
 *
 * @return 0 on success, iRODS error code on failure.
 */
int resolve_rods_path(rcComm_t *conn, rodsEnv *env,
                      rodsPath_t *rods_path, char *inpath);

/**
 * Initialise and set an iRODS path by copying a string into both its
 * inPath and outPath and then resolving it on the server. The path is not
 * parsed, so must be derived from an existing parsed path.
 *
 * @param[in]  conn      An open iRODS connection.
 * @param[in]  env       A populated iRODS environment.
 * @param[out] rodspath  An iRODS path.
 * @param[in]  path      A string representing an unresolved iRODS path.
 *
 * @return 0 on success, iRODS error code on failure.
 */
int set_rods_path(rcComm_t *conn, rodsPath_t *rods_path, char *path);

json_t *rods_path_to_json(rcComm_t *conn, rodsPath_t *rods_path);

json_t *list_path(rcComm_t *conn, rodsPath_t *rods_path, baton_error_t *error);

json_t *list_permissions(rcComm_t *conn, rodsPath_t *rods_path,
                         baton_error_t *error);

int modify_permissions(rcComm_t *conn, rodsPath_t *rods_path,
                       recursive_op recurse, char *owner_specifier,
                       char *access_level,  baton_error_t *error);

int modify_json_permissions(rcComm_t *conn, rodsPath_t *rods_path,
                            recursive_op recurse, json_t *perms,
                            baton_error_t *error);

/**
 * List metadata of a specified data object or collection.
 *
 * @param[in]  conn       An open iRODS connection.
 * @param[out] rodspath   An iRODS path.
 * @param[in]  attr_name  An attribute name to limit the values returned.
 *                        Optional, NULL means return all metadata.
 * @param[out] error      An error report struct.
 *
 * @return A newly constructed JSON array of AVU JSON objects.
 */
json_t *list_metadata(rcComm_t *conn, rodsPath_t *rods_path, char *attr_name,
                      baton_error_t *error);

/**
 * Search metadata to find matching data objects and collections.
 *
 * @param[in]  conn        An open iRODS connection.
 * @param[in]  query       A JSON query specification which includes the
 *                         attribute name and value to match. It may also have
 *                         an iRODS path to limit search scope. Only results
 *                          under this path will be returned. Omitting the path
 *                         means the search will be global.
 * @param[in]  zone_name   An iRODS zone name. Optional, NULL means the current
 *                         zone.
 * @param[out] error       An error report struct.
 *
 * @return A newly constructed JSON array of JSON result objects.
 */
json_t *search_metadata(rcComm_t *conn, json_t *query, char *zone_name,
                        baton_error_t *error);

/**
 * Apply a metadata operation to an AVU on a resolved iRODS path.
 *
 * @param[in]     conn        An open iRODS connection.
 * @param[in]     rodspath    A resolved iRODS path.
 * @param[in]     op          An operation to apply e.g. ADD, REMOVE.
 * @param[in]     attr_name   The attribute name.
 * @param[in]     attr_value  The attribute value.
 * @param[in]     attr_unit   The attribute unit (the empty string for none).
 * @param[in,out] error       An error report struct.
 *
 * @return 0 on success, iRODS error code on failure.
 */
int modify_metadata(rcComm_t *conn, rodsPath_t *rodspath, metadata_op op,
                    char *attr_name, char *attr_value, char *attr_unit,
                    baton_error_t *error);

/**
 * Apply a metadata operation to an AVU on a resolved iRODS path. The
 * functionality is identical to modify_metadata, except that the AVU is
 * a JSON struct argument, with optional units.
 *
 * @param[in]  conn        An open iRODS connection.
 * @param[in]  rodspath    A resolved iRODS path.
 * @param[in]  op          An operation to apply e.g. ADD, REMOVE.
 * @param[in]  avu         The JSON AVU.
 * @param[out] error       An error report struct.
 *
 * @return 0 on success, iRODS error code on failure.
 * @ref modify_metadata
 */
int modify_json_metadata(rcComm_t *conn, rodsPath_t *rods_path,
                         metadata_op operation, json_t *avu,
                         baton_error_t *error);
/**
 * Allocate a new iRODS generic query (see rodsGenQuery.h).
 *
 * @param[in] max_rows     Maximum number of rows to return.
 * @param[in] num_columns  The number of columns to select.
 * @param[in] columns      The columns to select.
 *
 * @return A pointer to a new genQueryInp_t which must be freed using
 * @ref free_query_input
 */
genQueryInp_t *make_query_input(int max_rows, int num_columns,
                                const int columns[]);
/**
 * Free memory used by an iRODS generic query (see rodsGenQuery.h).
 *
 * @param[in] query_in       The query to free.
 *
 * @ref make_query_input
 */
void free_query_input(genQueryInp_t *query_in);

genQueryInp_t *add_query_conds(genQueryInp_t *query_in, int num_conds,
                               const query_cond_t conds[]);

/**
 * Execute a general query and obtain results as a JSON array of objects.
 * Columns in the query are mapped to JSON object properties specified
 * by the labels argument.
 *
 * @param[in]  conn          An open iRODS connection.
 * @param[in]  query_in      A populated query input.
 * @param[in]  labels        An array of as many labels as there were columns
 *                           selected in the query.
 * @param[in,out] error      An error report struct.
 *
 * @return A newly constructed JSON array of objects, one per result row. The
 * caller must free this after use.
 */
json_t *do_query(rcComm_t *conn, genQueryInp_t *query_in,
                 const char *labels[], baton_error_t *error);

/**
 * Construct a JSON array of objects from a query output. Columns in the
 * query are mapped to JSON object properties specified by the labels
 * argument.
 *
 * @param[in] query_out     A populated query output.
 * @param[in] labels        An array of as many labels as there were columns
 *                          selected in the query.
 *
 * @return A newly constructed JSON array of objects, one per result row. The
 * caller must free this after use.
 */
json_t *make_json_objects(genQueryOut_t *query_out, const char *labels[]);

#endif // _BATON_H
