#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <stdlib.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sqlite3.h>

/*
 * populate table from a directory tree
 *
 * this is an example of building a table
 * from a recursive directory scan. it is
 * useful as a starting point - this demo
 * inserts only the filenames and an id
 *
 */

struct {
  int verbose;
  char *prog;
  enum {mode_build, mode_print} mode;

  /* file structure to index */
  char dir[PATH_MAX];
  int recurse;
  char *suffix;
  int file_id;

  /* db info */
  char *db_name;
  sqlite3 *db;
  sqlite3_stmt *insert_stmt;
  sqlite3_stmt *select_stmt;
  int truncate;


} CF = {
  .mode = mode_build,
  .recurse = 1,
  .truncate = 1,
};

void usage() {
  fprintf(stderr, "usage: %s [options] -b <dbfile>\n", CF.prog);
  fprintf(stderr, "\n");
  fprintf(stderr, "build mode (default):\n");
  fprintf(stderr, "   -d <dir>      (directory root to scan)\n");
  fprintf(stderr, "   -r [0|1]      (scan recursively; default: 1)\n");
  fprintf(stderr, "   -t [0|1]      (truncate db; default: 1)\n");
  fprintf(stderr, "   -s <suffix>   (only files matching suffix)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "print mode:\n");
  fprintf(stderr, "   -p            (print db)\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "general options:\n");
  fprintf(stderr, "   -v            (verbose, repeatable)\n");
  fprintf(stderr, "\n");
  exit(-1);
}

/* helper function for create statements that return no row */
int exec_sql(sqlite3 *db, char *sql) {
  sqlite3_stmt *ps=NULL;
  int sc, rc = -1;

  if (CF.verbose) fprintf(stderr, "executing SQL: %s\n", sql);

  sc = sqlite3_prepare_v2(db, sql, -1, &ps, NULL);
  if (sc != SQLITE_OK ){
    fprintf(stderr, "sqlite3_prepare: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  sc = sqlite3_step(ps);
  if (sc != SQLITE_DONE) {
    fprintf(stderr, "sqlite3_step: result unexpected\n");
    goto done;
  }

  sc = sqlite3_finalize(ps);
  if (sc != SQLITE_OK) {
    fprintf(stderr,"sqlite3_finalize: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  rc = 0;

 done:
  return rc;
}

int setup_db(void) {
  int sc, rc = -1;
  char *sql;

  sc = sqlite3_open(CF.db_name, &CF.db);
  if( sc ){
    fprintf(stderr, "sqlite3_open: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  /* create table */
  sql = "CREATE TABLE IF NOT EXISTS files (name TEXT PRIMARY KEY, id INTEGER);";
  if (exec_sql(CF.db, sql) < 0) goto done;

  /* truncate optionally */
  if ((CF.mode == mode_build) && CF.truncate) {
    sql = "DELETE FROM files;";
    if (exec_sql(CF.db, sql) < 0) goto done;
  }

  /* prepare insert statement - we substitute values in later */
  sql = "insert into files values ($NAME, $ID);";
  sc = sqlite3_prepare_v2(CF.db, sql, -1, &CF.insert_stmt, NULL);
  if( sc!=SQLITE_OK ){
    fprintf(stderr, "sqlite3_prepare: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  /* prepare select statement */
  sql = "select name, id from files;";
  sc = sqlite3_prepare_v2(CF.db, sql, -1, &CF.select_stmt, NULL);
  if( sc!=SQLITE_OK ){
    fprintf(stderr, "sqlite3_prepare: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  rc = 0;

 done:
  return rc;
}

int print_db(void) {
  int rc = -1, id, file_count=0;
  const unsigned char *name;

  while (sqlite3_step(CF.select_stmt) == SQLITE_ROW) {
    name = sqlite3_column_text(CF.select_stmt, 0);
    id = sqlite3_column_int(CF.select_stmt, 1);
    printf("%s %d\n", name, id);
    file_count++;
  }

  printf("%d files\n", file_count);

  rc = 0;

  return rc;
}

int reset(sqlite3_stmt *ps) {
  int sc, rc = -1;

  sc = sqlite3_reset(ps);
  if (sc != SQLITE_OK) {
    fprintf(stderr,"sqlite3_reset: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  sc = sqlite3_clear_bindings(ps);
  if (sc != SQLITE_OK) {
    fprintf(stderr,"sqlite3_clear_bindings: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  rc = 0;

 done:
  return rc;
}

int insert_file(sqlite3_stmt *ps, char *name, int id) {
  int sc, rc = -1;
  
  if (CF.verbose) fprintf(stderr, "inserting %s id %d\n", name, id);

  sc = sqlite3_bind_text(ps, 1, name, -1, SQLITE_TRANSIENT);
  if (sc != SQLITE_OK) {
    fprintf(stderr, "sqlite3_bind_text: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  sc = sqlite3_bind_int( ps, 2, id);
  if (sc != SQLITE_OK) {
    fprintf(stderr, "sqlite3_bind_int: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  /* insert */
  sc = sqlite3_step(ps);
  if (sc != SQLITE_DONE) {
    fprintf(stderr,"sqlite3_step: unexpected result\n");
    goto done;
  }

  if (reset(ps) < 0) goto done;

  rc = 0;

 done:
  return rc;
}

int add_file(char *file) {
  int rc = -1, sc;

  /* add file reference to file table */
  sc = insert_file(CF.insert_stmt, file, CF.file_id);
  if (sc < 0) goto done;
  CF.file_id++;
  
  rc = 0;

 done:
  return rc;
}

int is_suffix(char *file) {
  size_t file_len, suffix_len;
  char *file_suffix;

  /* not enforcing suffix match? */
  if (CF.suffix == NULL) return 1;

  file_len = strlen(file);
  suffix_len = strlen(CF.suffix);

  /* file too short for suffix match? */
  if (file_len < suffix_len) return 0;

  file_suffix = &file[ file_len - suffix_len ];
  return strcmp(file_suffix, CF.suffix) ? 0 : 1;
}

/* add directory to tree, recursively by option. 
 * function recursion depth bounded by fs depth
 *
 * returns
 *   < 0 on error
 *   0 success
 */
int add_dir(char *dir) {
  char path[PATH_MAX];
  struct dirent *dent;
  int rc = -1, ec;
  DIR *d = NULL;
  struct stat s;
  size_t l, el;

  if (CF.verbose) fprintf(stderr, "adding directory %s\n", dir);

  l = strlen(dir);
  d = opendir(dir);
  if (d == NULL) {
    fprintf(stderr, "opendir %s: %s\n", dir, strerror(errno));
    goto done;
  }

  /* iterate over directory contents. use stat to distinguish regular files
   * from directories (etc). stat is more robust than using dent->d_type */
  while ( (dent = readdir(d)) != NULL) {

    /* skip the . and .. directories */
    if (!strcmp(dent->d_name, "."))  continue;
    if (!strcmp(dent->d_name, "..")) continue;

    /* formulate path to dir entry */
    el = strlen(dent->d_name);
    if (l+1+el+1 > PATH_MAX) {
      fprintf(stderr, "path too long: %s/%s\n", dir, dent->d_name);
      goto done;
    }
    memcpy(path, dir, l);
    path[l] = '/';
    memcpy(&path[l+1], dent->d_name, el+1);

    /* lstat to determine its type */
    ec = lstat(path, &s);
    if (ec < 0) {
      fprintf(stderr, "lstat %s: %s\n", path, strerror(errno));
      goto done;
    }

    if (S_ISREG(s.st_mode) && is_suffix(path)) {
      if (add_file(path) < 0) goto done;
      if (CF.verbose) fprintf(stderr, "adding regular file %s\n", path);
    } 

    if (CF.recurse && (S_ISDIR(s.st_mode)))  {
      if (add_dir(path) < 0) goto done;
    }
  }

  rc = 0;

 done:
  if (d) closedir(d);
  return rc;
}

int release_db(void) {
  int sc, rc = -1;

  /* done with select statement */
  sc = sqlite3_finalize(CF.select_stmt);
  if (sc != SQLITE_OK) {
    fprintf(stderr,"sqlite3_finalize: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  /* done with insert statement */
  sc = sqlite3_finalize(CF.insert_stmt);
  if (sc != SQLITE_OK) {
    fprintf(stderr,"sqlite3_finalize: %s\n", sqlite3_errstr(sc));
    goto done;
  }

  sqlite3_close(CF.db); // NULL allowed

  rc = 0;

 done:
  return rc;
}

int main(int argc, char *argv[]) {
  char *dir = NULL;
  int opt, rc=-1;

  CF.prog = argv[0];

  while ( (opt = getopt(argc,argv,"vphd:r:t:s:b:")) > 0) {
    switch(opt) {
      case 'v': CF.verbose++; break;
      case 'd': dir = strdup(optarg); break;
      case 'r': CF.recurse=atoi(optarg); break;
      case 't': CF.truncate=atoi(optarg); break;
      case 'p': CF.mode = mode_print; break;
      case 's': CF.suffix = strdup(optarg); break;
      case 'b': CF.db_name = strdup(optarg); break;
      case 'h': default: usage(argv[0]); break;
    }
  }

  if (CF.db_name == NULL) usage();

  /* set up database */
  if (setup_db() < 0) goto done;

  switch (CF.mode) {
    case mode_build:
     if (dir == NULL) usage();
     if (realpath(dir, CF.dir) == NULL) {
       fprintf(stderr, "realpath %s: %s\n", dir, strerror(errno));
       goto done;
     }
     if (add_dir(CF.dir) < 0) goto done;
     break;
    case mode_print:
     if (print_db() < 0) goto done;
     break;
    default:
     assert(0);
     goto done;
     break;
  }

  rc = 0;
 
 done:
  if (dir) free(dir);
  if (CF.suffix) free(CF.suffix);
  if (CF.db_name) free(CF.db_name);
  release_db();
  return rc;
}
