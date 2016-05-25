/**
 * @file
 * @brief Header file for cfg_parse.
 *
 * This file should be included to use any features of cfg_parse. Typically, a user should
 * create a pointer to a cfg_struct, initialize it with cfg_init(), and then perform
 * actions on that object (lookup, add, delete) by passing the pointer to the functions here.
 * At end of use, call cfg_delete to clean up the object.
 */
#ifndef CFG_STRUCT_H_
#define CFG_STRUCT_H_

/* Declare C-style name mangling, this makes mixing with c++ compilers possible */
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Sets the maximum size of a line in a configuration file.
 * cfg_load uses this definition to limit the size of its read buffer.  Lines which exceed the
 * length do not crash outright, but probably won't load correctly.
 */
#define CFG_MAX_LINE 256

/* Opaque data structure holding config in memory */
struct cfg_struct;

/**
 * @brief Creates a cfg_struct.
 */
struct cfg_struct * cfg_init();

/**
 * @brief Frees a cfg_struct.
 */
void cfg_free(struct cfg_struct *);

/**
 * @brief Loads key=value pairs from a file into cfg_struct.
 */
int cfg_load(struct cfg_struct *, const char *, int);

/**
 * @brief Saves a cfg_struct to a file as key=value pairs.
 */
int cfg_save(const struct cfg_struct *, const char *);

/**
 * @brief Retrieves a value from a cfg_struct for a specified key.
 */
const char * cfg_get(const struct cfg_struct *, const char *);

/**
 * @brief Sets a key, value pair in a cfg_struct.
 */
void cfg_set(struct cfg_struct *, const char *, const char *);

/**
 * @brief Deletes a key (and associated value) from a cfg_struct.
 */
void cfg_delete(struct cfg_struct *, const char *);

#ifdef __cplusplus
}
#endif

#endif
