#include "relation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.tab.h"

enum attribute_modifier {
  empty_modifier,
  primary_key_modifier,
  foreign_key_modifier,
};

/* ???: typedef char attribute_id[5]; */

struct shallow_attribute {
  char *name;
  unsigned id;
  enum attribute_modifier modifier;
};

struct table {
  char *name;
  char *address;
  unsigned record_count;
  unsigned attribute_count;
  struct shallow_attribute *attributes;
};

struct unnamed_attribute {
  unsigned id;
  struct attribute_type type;
};

struct table_type {
  unsigned name;
  unsigned size;
  unsigned attribute_count;
  struct unnamed_attribute *attributes;
  unsigned table_count;
  struct table *tables;
};

struct table_type_list {
  struct table_type head;
  struct table_type_list *tail;
};

int main(void) {
  struct relation_list *relation_list;

  if (yyparse(&relation_list)) {
    return EXIT_FAILURE;
  }

  struct table_type_list *table_type_list = NULL;

  for (struct relation_list *relation_iterator = relation_list;
       relation_iterator; relation_iterator = relation_iterator->tail) {
    struct relation *relation = &relation_iterator->head;
    struct table_type_list *new_table_type_list =
        calloc(1, sizeof(struct table_type_list));
    struct table_type *table_type = &new_table_type_list->head;

    table_type->table_count = 1;

    for (struct attribute_list *attribute_iterator = relation->attribute_list;
         attribute_iterator; attribute_iterator = attribute_iterator->tail) {
      ++table_type->attribute_count;
    }

    table_type->attributes =
        calloc(table_type->attribute_count, sizeof(struct unnamed_attribute));

    unsigned i = 0;

    for (struct attribute_list *attribute_iterator = relation->attribute_list;
         attribute_iterator;
         attribute_iterator = attribute_iterator->tail, ++i) {
      table_type->attributes[i].id = i;
      table_type->attributes[i].type = attribute_iterator->head.type;
    }

    table_type->tables = calloc(1, sizeof(struct table));
    table_type->tables->name = relation->name;
    table_type->tables->address = relation->name;
    table_type->tables->attribute_count = table_type->attribute_count;
    table_type->tables->attributes =
        calloc(table_type->attribute_count, sizeof(struct shallow_attribute));

    i = 0;

    for (struct attribute_list *attribute_iterator = relation->attribute_list;
         attribute_iterator;
         attribute_iterator = attribute_iterator->tail, ++i) {
      table_type->tables->attributes[i].id = i;
      table_type->tables->attributes[i].name = attribute_iterator->head.name;
    }

    new_table_type_list->tail = table_type_list;
    table_type_list = new_table_type_list;
  }

  FILE *tdescr = fopen("tdescr.data", "w");

  for (struct table_type_list *table_type_iterator = table_type_list;
       table_type_iterator; table_type_iterator = table_type_iterator->tail) {
    struct table_type *table_type = &table_type_iterator->head;

    long position = ftell(tdescr);
    int has_next = -1;

    fwrite(&has_next, 4, 1, tdescr);
    fwrite(&table_type->name, 4, 1, tdescr);
    fwrite(&table_type->size, 4, 1, tdescr);
    fwrite(&table_type->attribute_count, 4, 1, tdescr);

    for (unsigned i = 0; i < table_type->attribute_count; ++i) {
      fwrite(&table_type->attributes[i].id, 4, 1, tdescr);
      fwrite("\0", 1, 1, tdescr);
      fwrite(&table_type->attributes[i].type.meta_type, 1, 1, tdescr);
      fwrite(&table_type->attributes[i].type.size, 1, 1, tdescr);
    }

    fwrite(&table_type->table_count, 4, 1, tdescr);

    for (unsigned i = 0; i < table_type->table_count; ++i) {
      unsigned name_length = strlen(table_type->tables[i].name);
      fwrite(&name_length, 4, 1, tdescr);
      fwrite(table_type->tables[i].name, 1, name_length, tdescr);

      unsigned address_length = strlen(table_type->tables[i].address);
      fwrite(&address_length, 4, 1, tdescr);
      fwrite(table_type->tables[i].address, 1, address_length, tdescr);

      fwrite(&table_type->tables[i].record_count, 4, 1, tdescr);
      fwrite(&table_type->tables[i].attribute_count, 4, 1, tdescr);

      for (unsigned j = 0; j < table_type->tables[i].attribute_count; ++j) {
        unsigned name_length = strlen(table_type->tables[i].attributes[j].name);
        fwrite(&name_length, 4, 1, tdescr);
        fwrite(table_type->tables[i].attributes[j].name, 1, name_length,
               tdescr);

        fwrite(&table_type->tables[i].attributes[j].id, 4, 1, tdescr);
        fwrite(&table_type->tables[i].attributes[j].modifier, 1, 1, tdescr);
      }
    }
  }

  fclose(tdescr);
}

void yyerror(struct relation_list **relation_list, const char *message) {
  fprintf(stderr, "%s\n", message);
}
