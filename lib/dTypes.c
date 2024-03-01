#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dTypes.h"

// -------------------------------------------------- Linked List ----------------------------------------------------

// function to create a new node
node_t *createNode(int data)
{
    node_t *newNode = (node_t *)malloc(sizeof(node_t));
    if (newNode == NULL)
    {
        printf("Error - Memory allocation failed\n");
        exit(1);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// function to insert a new node at the beginning of the list
void insertAtBeginning(node_t **headRef, int data)
{
    node_t *newNode = createNode(data);
    newNode->next = *headRef;
    *headRef = newNode;
}

// function to insert a new node at the end of the list
void insertAtEnd(node_t **headRef, int data)
{
    node_t *newNode = createNode(data);
    if (*headRef == NULL)
    {
        *headRef = newNode;
        return;
    }
    node_t *lastNode = *headRef;
    while (lastNode->next != NULL)
    {
        lastNode = lastNode->next;
    }
    lastNode->next = newNode;
}

// function to insert a new node after a given node
void insertAfter(node_t *prevNode, int data)
{
    if (prevNode == NULL)
    {
        printf("Error - Previous node cannot be NULL\n");
        return;
    }
    node_t *newNode = createNode(data);
    newNode->next = prevNode->next;
    prevNode->next = newNode;
}

// function to delete a node with given key
void deleteNode(node_t **headRef, int key)
{
    node_t *temp = *headRef;
    node_t *prev = NULL;
    if (temp != NULL && temp->data == key)
    {
        *headRef = temp->next;
        free(temp);
        return;
    }
    while (temp != NULL && temp->data != key)
    {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL)
        return;
    prev->next = temp->next;
    free(temp);
}

// function to print the linked list
void printList(node_t *head)
{
    node_t *current = head;
    while (current != NULL)
    {
        printf("%d -> ", current->data);
        current = current->next;
    }
    printf("NULL\n");
}

// ---------------------------------------------------- Hash Table ---------------------------------------------------

// hash function for hash map
unsigned int hash(char *key, int size)
{
    unsigned int hashval = 0;
    for (; *key != '\0'; key++)
    {
        hashval = *key + (hashval << 5) - hashval;
    }
    return hashval % size;
}

// function to create a new hash map
hashmap_t *createHashMap(int size)
{
    hashmap_t *map = (hashmap_t *)malloc(sizeof(hashmap_t));
    if (map)
    {
        map->size = size;
        map->table = (kv_t **)malloc(size * sizeof(kv_t *));
        if (map->table)
        {
            for (int i = 0; i < size; i++)
            {
                map->table[i] = NULL;
            }
        }
        else
        {
            free(map);
            return NULL;
        }
    }
    return map;
}

// function to insert a new key-value pair into the hash map
void insert(hashmap_t *map, char *key, int value)
{
    unsigned int index = hash(key, map->size);
    kv_t *newPair = (kv_t *)malloc(sizeof(kv_t));
    if (newPair)
    {
        newPair->key = strdup(key);
        newPair->value = value;
        newPair->next = NULL;
        if (map->table[index] == NULL)
        {
            map->table[index] = newPair;
        }
        else
        {
            kv_t *current = map->table[index];
            while (current->next != NULL)
            {
                current = current->next;
            }
            current->next = newPair;
        }
    }
}

// function to read the value of a key from the hash map
int get(hashmap_t *map, char *key)
{
    unsigned int index = hash(key, map->size);
    kv_t *pair = map->table[index];
    while (pair != NULL)
    {
        if (strcmp(pair->key, key) == 0)
        {
            return pair->value;
        }
        pair = pair->next;
    }
    return -1; // Key not found
}

// function to remove a key from the hash map
void removeKey(hashmap_t *map, char *key)
{
    unsigned int index = hash(key, map->size);
    kv_t *prev = NULL;
    kv_t *current = map->table[index];
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            if (prev == NULL)
            {
                map->table[index] = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            free(current->key);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// function to free the hash map
void freeHashMap(hashmap_t *map)
{
    if (map)
    {
        for (int i = 0; i < map->size; i++)
        {
            kv_t *current = map->table[i];
            while (current != NULL)
            {
                kv_t *temp = current;
                current = current->next;
                free(temp->key);
                free(temp);
            }
        }
        free(map->table);
        free(map);
    }
}

// ---------------------------------------------------- Queue ---------------------------------------------------

// function remove the first node from the queue
int dequeue(node_t **headRef)
{
    if (*headRef == NULL)
    {
        printf("Queue is empty!\n");
        exit(1);
    }
    int data = (*headRef)->data;
    node_t *temp = *headRef;
    *headRef = (*headRef)->next;
    free(temp);
    if (*headRef != NULL)
    {
        (*headRef)->prev = NULL;
    }
    return data;
}

// function to add a new node to the end of the queue
void enqueue(node_t **headRef, int data)
{
    insertAtEnd(headRef, data);
}