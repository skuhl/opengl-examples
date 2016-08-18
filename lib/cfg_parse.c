#include "windows-compat.h"
#include "cfg_parse.h"

/* for malloc, EXIT_SUCCESS and _FAILURE, exit */
#include <stdlib.h>
/* for FILE *, printf, etc */
#include <stdio.h>
/* for memset, strlen, strchr etc */
#include <string.h>
/* for tolower */
#include <ctype.h>
/* for errno in cfg_malloc */
#include <errno.h>

/* implementation details of (opaque) config structures */
struct cfg_node
{
  char *key;
  char *value;

  struct cfg_node *next;
};

struct cfg_struct
{
  struct cfg_node *head;
};

/* Helper functions
    A malloc() wrapper which handles null return values, and zeroes memory too */
static void *cfg_malloc(const unsigned int size)
{
  void *temp = malloc(size);
  if (temp == NULL && size != 0)
  {
    fprintf(stderr,"CFG_PARSE ERROR: MALLOC(%u) returned NULL (errno==",size);
    if (errno == ENOMEM)
      fprintf(stderr, "ENOMEM");
    else
      fprintf(stderr, "%d, ENOMEM==%d",errno,ENOMEM);
    fprintf(stderr,")\n");
    exit(EXIT_FAILURE);
  }
  memset(temp,0,size);
  return temp;
}

/* Determines if a character is a whitespace (blank) character or not. */
static char cfg_is_whitespace(const char c)
{
  return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

/* Returns a duplicate of input str, without leading / trailing whitespace
    Input str *MUST* be null-terminated, or disaster will result */
static char *cfg_trim(const char *str)
{
  int temp_len;
  char *tstr;

  /* check for null input first */
  if (str == NULL) return NULL;

  /* advance start pointer to first non-whitespace char */
  while (cfg_is_whitespace(*str))
    str ++;

  /* calculate length of output string, minus leading whitespace */
  temp_len = strlen(str);

  /* roll back length until we run out of whitespace */
  while (temp_len > 0 && cfg_is_whitespace(str[temp_len-1]))
    temp_len --;

  /* copy portion of string to new string */
  tstr = (char *)cfg_malloc(temp_len + 1);
  memcpy(tstr,str,temp_len);

  return tstr;
}

/**
 * This function loads data from a file, and inserts / updates the specified cfg_struct.
 * New keys will be inserted.  Existing keys can optionally have values overwritten by those read from the file.
 * The format of config-files is "key=value", with any amount of whitespace.
 * Comments can be included by using a # character: processing ends at that point.
 * The maximum line size is CFG_MAX_LINE-1 bytes (see cfg_parse.h)
 * @param cfg Pointer to cfg_struct to update.
 * @param filename String containing filename to open and parse.
 * @param overwrite If 1, existing values will be overwritten.
 * @return EXIT_SUCCESS (0) on success, or EXIT_FAILURE if file could not be opened.
 */
int cfg_load(struct cfg_struct *cfg, const char *filename, int overwrite)
{
  FILE *fp;
  char buffer[CFG_MAX_LINE], *delim;

  /* safety check: null input */
  if (cfg == NULL || filename == NULL) return EXIT_FAILURE;

  /* open file for reading */
  fp = fopen(filename, "r");
  if (fp == NULL) return EXIT_FAILURE;

  while (!feof(fp))
  {
    if (fgets(buffer,CFG_MAX_LINE,fp) != NULL)
    {
      /* locate first # sign and terminate string there (comment) */
      delim = strchr(buffer, '#');
      if (delim != NULL) *delim = '\0';

      /* locate first = sign and prepare to split */
      delim = strchr(buffer, '=');
      if (delim != NULL)
      {
        *delim = '\0';
        delim ++;

        if(overwrite == 1)
	        cfg_set(cfg,buffer,delim);
        else
        {
	        if(cfg_get(cfg, buffer) == NULL)
		        cfg_set(cfg,buffer,delim);
        }
      }
    }
  }

  fclose(fp);
  return EXIT_SUCCESS;
}

/**
 * This function saves a complete cfg_struct to a file.
 * Comments are not preserved.
 * @param cfg Pointer to cfg_struct to save.
 * @param filename String containing filename to open and parse.
 * @return EXIT_SUCCESS (0) on success, or EXIT_FAILURE if file could not be opened or a write error occurred.
 */
int cfg_save(const struct cfg_struct *cfg, const char *filename)
{
  FILE *fp;
  struct cfg_node *temp;

  /* safety check: null input */
  if (cfg == NULL || filename == NULL) return EXIT_FAILURE;

  /* open output file for writing */
  fp = fopen(filename, "w");
  if (fp == NULL) return EXIT_FAILURE;

  /* point at first item in list */
  temp = cfg->head;

  /* step through the list, dumping each key-value pair to disk */
  while (temp != NULL)
  {
    if (fprintf(fp,"%s=%s\n",temp->key,temp->value) < 0) { 
      fclose(fp);
      return EXIT_FAILURE;
    }
    temp = temp->next;
  }
  fclose(fp);
  return EXIT_SUCCESS;
}

/**
 * This function performs a key-lookup on a cfg_struct, and returns the associated value.
 * @param cfg Pointer to cfg_struct to search.
 * @param key String containing key to search for.
 * @return String containing associated value, or NULL if key was not found.
 */
const char * cfg_get(const struct cfg_struct *cfg, const char *key)
{
  unsigned int i, len;
  char *tkey;
  struct cfg_node *temp;

  /* safety check: null input */
  if (cfg == NULL || key == NULL) return NULL;

  /* Trim input search key */
  tkey = cfg_trim(key);

  /* Exclude empty key */
  if (! strcmp(tkey,"")) { free(tkey); return NULL; }

  /* Lowercase key */
  len = strlen(tkey);
  for (i = 0; i < len; i++)
    tkey[i] = tolower(tkey[i]);

  /* set up pointer to start of list */
  temp = cfg->head;

  while (temp != NULL)
  {
    if (! strcmp(tkey, temp->key))
    {
      free(tkey);
      return temp->value;
    }
    temp = temp->next;
  }

  free(tkey);
  return NULL;
}

/**
 * This function sets a single key-value pair in a cfg_struct.
 * If the key already exists, its value will be updated.
 * If not, a new item is added to the cfg_struct list.
 * There is a commented-out option to treat blank value as a delete operation:
 *  uncomment if your project needs this feature.
 * @param cfg Pointer to cfg_struct to search.
 * @param key String containing key to search for.
 * @param value String containing new value to assign to key.
 */
void cfg_set(struct cfg_struct *cfg, const char *key, const char *value)
{
  unsigned int i, len;
  char *tkey, *tvalue;
  struct cfg_node *temp;

  /* safety check: null input */
  if (cfg == NULL || key == NULL || value == NULL) return;

  /* Trim key. */
  tkey = cfg_trim(key);
  /* Exclude empty key */
  if (! strcmp(tkey,"")) { free(tkey); return; }

  /* Lowercase key */
  len = strlen(tkey);
  for (i = 0; i < len; i++)
    tkey[i] = tolower(tkey[i]);

  /* Trim value. */
  tvalue = cfg_trim(value);

  /* Depending on implementation, you may wish to treat blank value
     as a "delete" operation */
  /* if (! strcmp(tvalue,"")) { free(tvalue); cfg_delete(cfg,tkey); free(tkey); return; } */

  /* point at first item in list */
  temp = cfg->head;

  /* search list for existing key */
  while (temp != NULL)
  {
    if (! strcmp(tkey, temp->key))
    {
      /* found a match: no longer need temp key */
      free(tkey);

      /* update value */
      free(temp->value);
      temp->value = tvalue;
      return;
    }
    temp = temp->next;
  }

  /* not found: create new element */
  temp = (struct cfg_node *)cfg_malloc(sizeof(struct cfg_node));

  /* assign key, value */
  temp->key = tkey;
  temp->value = tvalue;

  /* prepend */
  temp->next = cfg->head;
  cfg->head = temp;
}

/**
 * This function deletes a key-value pair from a cfg_struct.
 * If the key does not exist, the function does nothing.
 * @param cfg Pointer to cfg_struct to search.
 * @param key String containing key to search for.
 */
void cfg_delete(struct cfg_struct *cfg, const char *key)
{
  unsigned int i, len;
  char *tkey;
  struct cfg_node *temp, *temp2 = NULL;

  /* safety check: null input */
  if (cfg == NULL || key == NULL) return;

  /* trim input key */
  tkey = cfg_trim(key);
  /* Exclude empty key */
  if (! strcmp(tkey,"")) { free(tkey); return; }

  /* Lowercase key */
  len = strlen(tkey);
  for (i = 0; i < len; i++)
    tkey[i] = tolower(tkey[i]);

  /* set pointer to start of list */
  temp = cfg->head;

  /* search list for existing key */
  while (temp != NULL)
  {
    if (! strcmp(tkey, temp->key))
    {
      /* cleanup trimmed key */
      free(tkey);

      if (temp == cfg->head)
      {
        /* first element */
        cfg->head = temp->next;
      } else {
        /* splice out element */
        temp2->next = temp->next;
      }

      /* delete element */
      free(temp->value);
      free(temp->key);
      free(temp);

      return;
    }

    temp2 = temp;
    temp = temp->next;
  }

  /* not found */
  /* cleanup trimmed key */
  free(tkey);
}

/**
 * This function initializes a cfg_struct, and must be called before
 * performing any further operations.
 * @return Pointer to newly initialized cfg_struct object.
 */
struct cfg_struct * cfg_init()
{
  struct cfg_struct *temp;
  temp = (struct cfg_struct *)cfg_malloc(sizeof(struct cfg_struct));
  temp->head = NULL;
  return temp;
}

/**
 * This function deletes an entire cfg_struct, clearing any memory
 * previously held by the structure.
 * @param cfg Pointer to cfg_struct to delete.
 */
void cfg_free(struct cfg_struct *cfg)
{
  struct cfg_node *temp = NULL, *temp2;
  temp = cfg->head;
  while (temp != NULL)
  {
    temp2 = temp->next;
    free(temp->key);
    free(temp->value);
    free(temp);
    temp = temp2;
  }
  free (cfg);
}
