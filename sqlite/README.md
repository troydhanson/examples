See https://www.sqlite.org

Sqlite is probably installed on your Linux host already. You'll need to add the
development headers to compile to the C API. 

On a typical yum based host:

    yum install sqlite
    yum install sqlite-devel

Or on Ubuntu using apt:

    apt install sqlite3
    apt install libsqlite3-dev

## Sqlite3

The sqlite3 binary can be used on the command line. This creates example.db:

    % sqlite] sqlite3 example.db
    sqlite> create table people (name text, age integer);

    sqlite> .tables
    people

    sqlite> .schema people
    CREATE TABLE people (name text, age integer);

    sqlite> insert into people values ("ben", 8);
    sqlite> insert into people values ("sarah", 5);

    sqlite> select * from people;
    ben|8
    sarah|5
    sqlite> .exit

## C API

See simple.c in this directory. It is an example from the SQLite documentation.
Run "make" to compile simple from simple.c.

    ./simple example.db "select * from people;"
    name = ben
    age = 8

    name = sarah
    age = 5

The simple.c example uses the convenience wrapper `sqlite3_exec`.

### C examples

The programs `create.c`, `insert.c`, and `select.c` show basic C API usage.

  % make
  % rm -f example.db
  % ./create example.db
  % ./insert example.db
  % ./select example.db
  name: ben age 8
  name: isaac age 13

#### Directory scan example

There is an example of scanning a directory tree and populating a table of
filenames in `scan_files.c`. The first usage builds the database. The second
prints it (`-p`).

    ./scan_files -b files.db -d /usr/share/dict
    ./scan_files -b files.db -p

The example `scan_lines.c` takes it a step further, scanning all the files
it finds in the directory tree, producing a table of line offsets in each file.
(In the resulting records, sequences of newlines are reduced to one record,
and leading or trailing newlines in the file are ignored).

    ./scan_lines -b tmp.db -d /usr/share/dict
    ./scan_lines -b tmp.db -p | head

## SQLite Documentation

 * Core C API: https://www.sqlite.org/cintro.html
 * SQL language: https://www.sqlite.org/lang.html 
 * Datatypes: https://www.sqlite.org/datatype3.html

## Lessons learned

### Proceed to `SQLITE_DONE`

Always let `sqlite3_step` progress until it returns `SQLITE_DONE`- even if
the query can only ever return one row. The memory used to get the columns from
a row is held until the row is stepped to the next row (or to `SQLITE_DONE`).
Furthermore other queries can eventually show the table as locked unless you
release it by stepping all the way to `SQLITE_DONE`.

### Increase busy timeout 

Increase the timeout for lock acquisition. I use 10 seconds. 

    sqlite3_busy_timeout(db, 10000);

Otherwise the application can give up on the lock so fast, that even a brief
competing query from a user's sqlite3 session can disrupt the application.

### Use "IF NOT EXISTS" and "OR REPLACE"

It's easy to create a table and its indexes if they don't already exist:

    CREATE TABLE IF NOT EXISTS ...
    CREATE INDEX IF NOT EXISTS ...

### Use "OR REPLACE"

Replacing a row (where primary key of the insert matches an existing row) can
be done as a REPLACE instead of a DELETE/INSERT.

    INSERT OR REPLACE INTO ...

### Bind indexes are 1-based, column indexes are 0-based

    sqlite3_bind_text(ps_del, 1, ...); # bind first value (1)
    sqlite3_column_int64(ps_query, 0); # get first column (0)

## Examples

## Basic

A few examples of SQL to jog the memory:

    CREATE TABLE IF NOT EXISTS files (name TEXT PRIMARY KEY, size INTEGER);
    CREATE INDEX IF NOT EXISTS bysize ON files(size);
    DELETE FROM files WHERE name = "blue";
    SELECT SUM(size) from files;
    INSERT OR REPLACE INTO files VALUES ("red", 100);

A multi-column primary key:

    CREATE TABLE lines (id INTEGER, pos INTEGER, sortkey INTEGER, 
                        CONSTRAINT pk PRIMARY KEY (id, pos));

## Join

    CREATE TABLE files (name TEXT PRIMARY KEY, id INTEGER);
    CREATE TABLE lines (id INTEGER, pos INTEGER, sortkey INTEGER);
    SELECT f.name, l.pos, l.sortkey FROM files f, lines l WHERE l.id = f.id

### Join with group by

    SELECT l.id, name, count(*) 
    FROM lines l, files f 
    WHERE l.id = f.id 
    GROUP BY l.id;

