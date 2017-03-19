/** @file hash_table.h
 *  @brief This file declares the the hash table structure as well as functions
 *   to use the generic hash table.
 *  @author akanjani, lramire1
 */

#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <linked_list.h>

/** @brief A structure that represents a hash table
 */
typedef struct hash_table {

  /** @brief A pointer to generic_linked_list_t type which will be used as an
   *   array where each memeber forms a bucket for this hash table
   */
  generic_linked_list_t *buckets;

  /** @brief An unsigned storing the number of buckets for this hash table
   */
  unsigned int nb_buckets;

  /** @brief A function pointer to the hash function for this hash table
   */
  unsigned int (*hash_function)(void *elem, unsigned int nb_buckets);

  /** @brief A function pointer to the function to find an element in the
   *   hash table
   */
  int (*find)(void *elem, void *value);

} generic_hash_table_t;

int hash_table_init(generic_hash_table_t *hash_table, unsigned int nb_buckets,
                    int (*find)(void *, void *),
                    unsigned int (*hash_function)(void *, unsigned int));
int hash_table_add_element(generic_hash_table_t *hash_table, void *elem);
void *hash_table_remove_element(generic_hash_table_t *hash_table, void *elem);
void *hash_table_get_element(generic_hash_table_t *hash_table, void *elem);

#endif /* _HASH_TABLE_H */
