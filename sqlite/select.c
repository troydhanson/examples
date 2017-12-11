#include <stdio.h>
#include <sqlite3.h>

/*
   This example of using sqlite uses the standard function sequence:
				open/prepare/step/close

   To try it, first create the example.db with this content:

    % sqlite3 example.db
    sqlite> create table people (name text, age integer);
    sqlite> insert into people values ("ben", 8);
    sqlite> insert into people values ("sarah", 5);
    sqlite> .exit

   Then run this program:

   % ./select example.db

 */

int dump_columns(sqlite3_stmt *ppStmt) {
  const unsigned char *name;
  int age;
  name = sqlite3_column_text(ppStmt, 0);
  age = sqlite3_column_int(ppStmt, 1);
  printf("name: %s age %d\n", name, age);
  return 0;
}

int main(int argc, char **argv){
  sqlite3 *db=NULL;
  sqlite3_stmt *ppStmt;
  int rc=-1, sc, more;

  if( argc<2 ){
    fprintf(stderr, "Usage: %s DATABASE [SQL-STATEMENT]\n", argv[0]);
    goto done;
  }

  sc = sqlite3_open(argv[1], &db);
  if( sc ){
    fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errmsg(db));
    goto done;
  }

  char *sql = (argc > 2) ? argv[2] : "SELECT * from people;";
  sc = sqlite3_prepare_v2(db, sql, -1, &ppStmt, NULL);
  if( sc!=SQLITE_OK ){
    fprintf(stderr, "sqlite3_prepare: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  more = 1;
  while(more) {
    sc = sqlite3_step(ppStmt);
    switch(sc) {
      case SQLITE_ROW: dump_columns(ppStmt); break;
      case SQLITE_DONE: 
        more = 0;
        sc = sqlite3_finalize(ppStmt);
        if (sc != SQLITE_OK) {
          fprintf(stderr,"sqlite3_finalize: %s\n", sqlite3_errstr(sc));
        }
        break;
      default: 
        fprintf(stderr,"sqlite3_step: error %d\n", sc);
        more = 0;
        break;
    }
  }

  rc = 0;

 done:
  sqlite3_close(db); // NULL allowed
  return rc;
}
