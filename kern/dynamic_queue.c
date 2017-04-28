/** @file dynamic_queue.c
 *  @brief This file contains the definitions for functions which can be used
 *         to insert or delete elements from the generic queue
 *  @author akanjani, lramire1
 */

#include <dynamic_queue.h>
#include <stdlib.h>
#include <eff_mutex.h>
#include <assert.h>

/** @brief  Makes a generic_node_t type node to be added in the queue
 *
 *  @param  value The value to be stored in the list casted as void*
 *
 *  @return A pointer to the generic_node_t type new node on success, NULL
 *          otherwise
 */
static generic_node_t *make_node(void *value) {

  // Allocate space for the new node
  generic_node_t *new_node = malloc(sizeof(generic_node_t));
  if (new_node == NULL) {
    return NULL;
  }

  // Set the appropriate values for the node
  new_node->value = value;
  new_node->next = NULL;

  return new_node;
}

/** @brief  Initializes the queue
 *
 *  @param  list The queue to be initialized
 *
 *  @return 0 on success, a negative error code on failure
 */
int queue_init(generic_queue_t *list) {

  // Check argument
  if (list == NULL) {
    return -1;
  }

  // Initialize the head and tail to NULL
  list->head = list->tail = NULL;

  // Initialize the mutex
  if (eff_mutex_init(&list->mp) < 0) {
    return -1;
  }

  return 0;
}

/** @brief  Inserts a generic_node_t type node at the tail end of the queue
 *
 *  @param  value   The value to be stored in the list casted as void*
 *  @param  list    A pointer to a generic_queue_t structure which holds
 *                  the head and tail pointer of a queue
 *
 *  @return 0 on success, a negative error code on failure
 */
int queue_insert_node(generic_queue_t *list, void *value) {

  // Check argument
  assert(list != NULL);

  // Make a new node
  generic_node_t *new_node = make_node(value);

  if (!new_node) {
    // Error creating a new node
    return -1;
  }

  // Acquire eff_mutex
  eff_mutex_lock(&list->mp);

  generic_node_t **head = &list->head;
  generic_node_t **tail = &list->tail;

  if (!head || !tail) {
    // Invalid double pointer
    eff_mutex_unlock(&list->mp);
    free(new_node);
    return -1;
  }

  if (*tail == NULL && *head == NULL) {
    // head is NULL. This is the first element of the list
    *tail = new_node;
    *head = new_node;
    eff_mutex_unlock(&list->mp);
    return 0;
  }

  // Add the new node to the end of the list
  (*tail)->next = new_node;
  *tail = new_node;

  // Release mutex
  eff_mutex_unlock(&list->mp);

  return 0;
}

/** @brief  Deletes a generic_node_t type node from the front end of the queue
 *
 *  @param  list  A pointer to a generic_queue_t structure which holds
 *                the head and tail pointer of a queue
 *
 *  @return The value of the element in the deleted node cast as a void* on
 *          success (the queue was non-empty), NULL otherwise
 *              
 */
void *queue_delete_node(generic_queue_t *list) {

  // Acquire mutex
  eff_mutex_lock(&list->mp);

  generic_node_t **head = &list->head;
  generic_node_t **tail = &list->tail;

  if (!head || !tail) {
    // Invalid double pointer
    eff_mutex_unlock(&list->mp);
    return NULL;
  }

  if (*head == NULL || *tail == NULL) {
    // list is empty or the tail/head pointer is messed up
    eff_mutex_unlock(&list->mp);
    return NULL;
  }

  if (*head == *tail) {
    // The only element in the list
    *tail = NULL;
  }

  // Store the current head
  generic_node_t *tmp = *head;
  void *ret = (*head)->value;

  // Update head
  *head = (*head)->next;

  // Release mutex
  eff_mutex_unlock(&list->mp);

  // Free the space for deleted node
  free(tmp);
  tmp = NULL;

  return ret;
}

/** @brief  Checks if the queue is empty or not
 *
 *  @param  list  A pointer to a generic_queue_t structure which holds
 *                the head and tail pointer of a queue
 *
 *  @return 1 is the list is empty, 0 otherwise
 */
int is_queue_empty(generic_queue_t *list) {

  // Acquire mutex
  eff_mutex_lock(&list->mp);
  
  if ( list->head == NULL ) {
    // List is empty. Release mutex
    eff_mutex_unlock(&list->mp);
    return 1;
  }

  // Release mutex
  eff_mutex_unlock(&list->mp);
  return 0;
}
