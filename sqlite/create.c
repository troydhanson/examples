#include <stdio.h>
#include <sqlite3.h>

/*
  an example of creating a table using SQL's CREATE table

 */

int exec_sql(sqlite3 *db, char *sql) {
  sqlite3_stmt *ppStmt=NULL;
  int sc, rc = -1;

  sc = sqlite3_prepare_v2(db, sql, -1, &ppStmt, NULL);
  if( sc!=SQLITE_OK ){
    fprintf(stderr, "sqlite3_prepare: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  sc = sqlite3_step(ppStmt);
  switch(sc) {
    case SQLITE_ROW: 
      fprintf(stderr, "more results -- unexpected\n");
      break;
    case SQLITE_DONE: 
      break;
    default: 
      fprintf(stderr,"sqlite3_step: error %s\n", sqlite3_errstr(sc));
      goto done;
      break;
  }

  sc = sqlite3_finalize(ppStmt);
  if (sc != SQLITE_OK) {
    fprintf(stderr,"sqlite3_finalize: %s\n", sqlite3_errstr(sc));
    goto done;
  }


  rc = 0;

 done:
  return rc;
}

int main(int argc, char **argv){
  int rc=-1, sc;
  sqlite3 *db=NULL;
  char *sql;

  if( argc<2 ){
    fprintf(stderr, "Usage: %s DATABASE\n", argv[0]);
    goto done;
  }

  sc = sqlite3_open(argv[1], &db);
  if( sc ){
    fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  sql = "CREATE TABLE people (name TEXT PRIMARY KEY, age INTEGER);";
  if (exec_sql(db, sql) < 0) goto done;

  rc = 0;

 done:
  sqlite3_close(db); // NULL allowed
  return rc;
}
