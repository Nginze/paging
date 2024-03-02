#ifndef D_TYPES_H
#define D_TYPES_H

// ------------------------------------ Linked List ----------------------------------------

/*
    Custom type definition for doubly linked list
*/
typedef struct Node
{
    int data;
    struct Node *next;
    struct Node *prev;
} node_t;

typedef enum
{
    false = 0,
    true = !false
} bool;

// function to create a new node
node_t *createNode(int data);

// function to insert a new node at the beginning of the list
void insertAtBeginning(node_t **headRef, int data);

// function to insert a new node at the end of the list
void insertAtEnd(node_t **headRef, int data);

// function to insert a new node after a given node
void insertAfter(node_t *prevNode, int data);

// function to delete a node from the list
void deleteNode(node_t **headRef, int key);

// function to print the list
void printList(node_t *head);

// ------------------------------------ Hash Table ------------------------------------------

/*
    Custom type definition for key-value pair
*/
typedef struct KeyValuePair
{
    char *key;
    int value;
    struct KeyValuePair *next;
} kv_t;

/*
    Custom type definition for hash map
*/
typedef struct HashMap
{
    int size;
    kv_t **table;
} hashmap_t;

// function to create a new hash map
hashmap_t *createHashMap(int size);

// function to insert a new key-value pair into the hash map
void insert(hashmap_t *map, char *key, int value);

// function to read the value of a key from the hash map
int get(hashmap_t *map, char *key);

// function to remove a key from the hash map
void removeKey(hashmap_t *map, char *key);

// function to free the hash map
void freeHashMap(hashmap_t *map);

// -------------------------------------- Queue -----------------------------------------------

// function to dequeue (remove from the beginning) of the queue
int dequeue(node_t **headRef);

// function to enqueue (add to the end) of the queue
void enqueue(node_t **headRef, int data);

#endif
