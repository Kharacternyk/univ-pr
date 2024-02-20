Компіляція:

```bash
bison -d parser.y
flex lexer.l
gcc -lfl main.c parser.tab.c lex.yy.c
```

`main.c` містить незавершену реалізацію запису в файл `tdescr`.
Ця незавершена реалізація може бути прикладом використання структур з `relation.h`,
що створюються парсером.
