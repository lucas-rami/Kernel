/** @file hash_table.c
 *
 *  @brief  This file contains the definitions for functions which can be used
 *          to manipulate the generic hash table with elements of type void*
 *
 *  @author akanjani, lramire1
 */

#include <hash_table.h>
#include <linked_list.h>
#include <stdlib.h>

/* Debugging */
#include <simics.h>

/** @brief  Initializes the hash table
 *
 *  The function must be called once before any other function in this file,
 *  otherwise the hash table's behavior is undefined.
 *
 *  @param  hash_table     The hash table to initialize
 *  @param  nb_buckets     The number of buckets in the hash table
 *  @param  find           Function to find a particular element in the list
 *  @param  hash_function  A hashing function
 *
 *  @return 0 on success, a negative error code on failure
 */
int hash_table_init(generic_hash_table_t *hash_table, unsigned int nb_buckets,
                    int (*find)(void *, void *),
                    unsigned int (*hash_function)(void *, unsigned int)) {

  // Check validity of arguments
  if (hash_table == NULL || nb_buckets <= 0 || find == NULL || 
      hash_function == NULL) {
    return -1;
  }

  hash_table->nb_buckets = nb_buckets;
  hash_table->hash_function = hash_function;

  // Allocate the buckets
  hash_table->buckets =
      calloc(hash_table->nb_buckets, sizeof(generic_linked_list_t));
  if (hash_table->buckets == NULL) {
    return -1;
  }

  // Initialize the hash table
  int i;
  for (i = 0; i < hash_table->nb_buckets; ++i) {

    // Initialize each list
    if (linked_list_init(&hash_table->buckets[i], find) < 0) {
      free(hash_table->buckets);
      return -1;
    }

  }

  return 0;
}

/** @brief  Adds an element to the hash table
 *
 *  @param  hash_table The hash table
 *  @param  elem       The element to add
 *
 *  @return 0 on success, a negative error code on failure
 */
int hash_table_add_element(generic_hash_table_t *hash_table, void *elem) {

  // Check validity of arguments
  if (hash_table == NULL || hash_table->hash_function == NULL || elem == NULL)
  {
    return -1;
  }

  // Compute in which bucket the element will go
  unsigned bucket = hash_table->hash_function(elem, hash_table->nb_buckets);

  if (bucket < 0 || bucket >= hash_table->nb_buckets) {
    return -1;
  }

  // Add the element to the appropriate linked list
  if (linked_list_insert_node(&hash_table->buckets[bucket], elem) < 0) {
    return -1;
  }

  return 0;
}

/** @brief  Removes an element in the hash table
 *
 *  @param  hash_table   A hash table
 *  @param  elem         The element to remove
 *
 *  @return The deleted element's value if it was found in the list.
 *          NULL otherwise
 */
void *hash_table_remove_element(generic_hash_table_t *hash_table, void *elem) {

  // Check validity of arguments
  if (hash_table == NULL || hash_table->hash_function == NULL || elem == NULL)
  {
    return NULL;
  }

  // Compute in which bucket the element could be
  unsigned bucket = hash_table->hash_function(elem, hash_table->nb_buckets);

  if (bucket < 0 || bucket >= hash_table->nb_buckets) {
    return NULL;
  }

  return linked_list_delete_node(&hash_table->buckets[bucket], elem);
}

/** @brief  Gets an element in the hash table
 *
 *  @param  hash_table   A hash_table
 *  @param  elem         The element to get
 *
 *  @return The element if it was found in the hash table, NULL otherwise.
 */
void *hash_table_get_element(generic_hash_table_t *hash_table, void *elem) {

  // Check validity of arguments
  if (hash_table == NULL || hash_table->hash_function == NULL || elem == NULL)
  {
    return NULL;
  }

  // Compute in which bucket the element could be
  unsigned bucket = hash_table->hash_function(elem, hash_table->nb_buckets);

  if (bucket < 0 || bucket >= hash_table->nb_buckets) {
    return NULL;
  }

  return linked_list_get_node(&hash_table->buckets[bucket], elem);
}
