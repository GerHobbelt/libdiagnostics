
#pragma once

#include <diagnostics/assertions.h>



















// Please note adding the fields to serialize as shown above is very poor (since this format cannot be unserialized).
// To deal with blobs, you need to find a good serialization library or format (protocol buffer, message pack, JSON, etc ...) or roll your own.


















/*
** This file contains an example of how to use SQLite to store and retrieve
** blob data. Blobs are stored in the database indexed by a string key,
** using a table declared as:
**
**   CREATE TABLE blobs(
**       key TEXT PRIMARY KEY,
**       value BLOB
**   );
**
** The following functions are implemented to interface with the database:
**
** createBlobTable() - Create the blobs table in the database.
** writeBlob()       - Store a blob in the database.
** readBlob()        - Read a blob out of the database.
** freeBlob()        - Free memory allocated by a call to readBlob().
**
** Also included is a main() routine that uses the above API to implement
** a (silly) file archive program. The file archive program can read files
** from the file-system and store them as blobs in a database, or it can
** read blobs from the database and store them as files in the file-system.
**
** Usage:
**
**   a.out <database-name> store <file-name>
**
** Read file <file-name> and store it as a blob in database <database-name>
**
**   a.out <database-name> retrieve <file-name>
**
** Read file <file-name> from database <database-name> and write it into
** the file-system.
**
** Example:
**
** $ cp /bin/sh .
** $ ./a.out mydb.db store sh
** $ rm sh
** $ ./a.out mydb.db retrieve sh
** $ diff /bin/sh sh
** $
**
*/

#include <sqlite3.h>
#include <assert.h>
#include <malloc.h>

/*
** Create the blobs table in database db. Return an SQLite error code.
*/
static int createBlobTable(sqlite3 *db) {
	const char *zSql = "CREATE TABLE blobs(key TEXT PRIMARY KEY, value BLOB)";
	return sqlite3_exec(db, zSql, 0, 0, 0);
}

/*
** Store a blob in database db. Return an SQLite error code.
**
** This function inserts a new row into the blobs table. The 'key' column
** of the new row is set to the string pointed to by parameter zKey. The
** blob pointed to by zBlob, size nBlob bytes, is stored in the 'value'
** column of the new row.
*/
static int writeBlob(
  sqlite3 *db,                   /* Database to insert data into */
  const char *zKey,              /* Null-terminated key string */
  const unsigned char *zBlob,    /* Pointer to blob of data */
  int nBlob                      /* Length of data pointed to by zBlob */
) {
	const char *zSql = "INSERT INTO blobs(key, value) VALUES(?, ?)";
	sqlite3_stmt *pStmt;
	int rc;

	do {
		/* Compile the INSERT statement into a virtual machine. */
		rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
		if (rc!=SQLITE_OK) {
			return rc;
		}

		/* Bind the key and value data for the new table entry to SQL variables
		** (the ? characters in the sql statement) in the compiled INSERT
		** statement.
		**
		** NOTE: variables are numbered from left to right from 1 upwards.
		** Passing 0 as the second parameter of an sqlite3_bind_XXX() function
		** is an error.
		*/
		sqlite3_bind_text(pStmt, 1, zKey, -1, SQLITE_STATIC);
		sqlite3_bind_blob(pStmt, 2, zBlob, nBlob, SQLITE_STATIC);

		/* Call sqlite3_step() to run the virtual machine. Since the SQL being
		** executed is not a SELECT statement, we assume no data will be returned.
		*/
		rc = sqlite3_step(pStmt);
		assert(rc!=SQLITE_ROW);

		/* Finalize the virtual machine. This releases all memory and other
		** resources allocated by the sqlite3_prepare() call above.
		*/
		rc = sqlite3_finalize(pStmt);

		/* If sqlite3_finalize() returned SQLITE_SCHEMA, then try to execute
		** the statement again.
		*/
	} while (rc==SQLITE_SCHEMA);

	return rc;
}

/*
** Read a blob from database db. Return an SQLite error code.
*/
static int readBlob(
  sqlite3 *db,               /* Database containing blobs table */
  const char *zKey,          /* Null-terminated key to retrieve blob for */
  unsigned char **pzBlob,    /* Set *pzBlob to point to the retrieved blob */
  int *pnBlob                /* Set *pnBlob to the size of the retrieved blob */
) {
	const char *zSql = "SELECT value FROM blobs WHERE key = ?";
	sqlite3_stmt *pStmt;
	int rc;

	/* In case there is no table entry for key zKey or an error occurs,
	** set *pzBlob and *pnBlob to 0 now.
	*/
	*pzBlob = 0;
	*pnBlob = 0;

	do {
		/* Compile the SELECT statement into a virtual machine. */
		rc = sqlite3_prepare(db, zSql, -1, &pStmt, 0);
		if (rc!=SQLITE_OK) {
			return rc;
		}

		/* Bind the key to the SQL variable. */
		sqlite3_bind_text(pStmt, 1, zKey, -1, SQLITE_STATIC);

		/* Run the virtual machine. We can tell by the SQL statement that
		** at most 1 row will be returned. So call sqlite3_step() once
		** only. Normally, we would keep calling sqlite3_step until it
		** returned something other than SQLITE_ROW.
		*/
		rc = sqlite3_step(pStmt);
		if (rc==SQLITE_ROW) {
			/* The pointer returned by sqlite3_column_blob() points to memory
			** that is owned by the statement handle (pStmt). It is only good
			** until the next call to an sqlite3_XXX() function (e.g. the
			** sqlite3_finalize() below) that involves the statement handle.
			** So we need to make a copy of the blob into memory obtained from
			** malloc() to return to the caller.
			*/
			*pnBlob = sqlite3_column_bytes(pStmt, 0);
			*pzBlob = (unsigned char *)malloc(*pnBlob);
			memcpy(*pzBlob, sqlite3_column_blob(pStmt, 0), *pnBlob);
		}

		/* Finalize the statement (this releases resources allocated by
		** sqlite3_prepare() ).
		*/
		rc = sqlite3_finalize(pStmt);

		/* If sqlite3_finalize() returned SQLITE_SCHEMA, then try to execute
		** the statement all over again.
		*/
	} while (rc==SQLITE_SCHEMA);

	return rc;
}

/*
** Free a blob read by readBlob().
*/
static void freeBlob(unsigned char *zBlob) {
	free(zBlob);
}

/*************************************************************************
** The code below this point is the test program that uses the above API.
** It's not all that illustrative. But it proves the above is more or
** less correct.
**
*/
/* #define OMIT_MAIN_FUNCTION */
#ifndef OMIT_MAIN_FUNCTION

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
/*
** Print the most recent database error for database db to standard error.
*/
static void databaseError(sqlite3* db) {
	int errcode = sqlite3_errcode(db);
	const char *errmsg = sqlite3_errmsg(db);
	fprintf(stderr, "Database error %d: %s\n", errcode, errmsg);
}
int main(int argc, char **argv) {
	sqlite3 *db;                  /* Database connection */
	char const *zDatabase;        /* Database file name */
	char const *zFile;            /* File to store/retrieve */
	int isStore = 0;              /* 1 for store, 0 for retrieve */

	/* Interpret command line arguments. */
	if (argc!=4) {
		fprintf(stderr, "Usage: %s <db file> store|retrieve <filename>\n", argv[0]);
		return 1;
	}
	zDatabase = argv[1];
	zFile = argv[3];
	if (argv[2][0]=='s' || argv[2][0]=='S') {
		isStore = 1;
	}

	/* Open the database */
	sqlite3_open(zDatabase, &db);
	if (SQLITE_OK!=sqlite3_errcode(db)) {
		databaseError(db);
		return 1;
	}

	/* Create the blobs table if it has not already been created. We ignore
	** the error code returned by this call. An error probably just means
	** the blobs table has already been created anyway.
	*/
	createBlobTable(db);

	if (isStore) {
		/* A store operation. Read in the file zFile and store it as a blob
		** in the database.
		*/
		int fd;
		struct stat sStat;
		int nBlob;
		unsigned char *zBlob;

		/* Open the file. */
		fd = open(zFile, O_RDONLY);
		if (fd<0) {
			perror("open");
			return 1;
		}

		/* Find out the file size. */
		if (0!=fstat(fd, &sStat)) {
			perror("fstat");
			return 1;
		}
		nBlob = sStat.st_size;

		/* Read the file data into zBlob. Close the file. */
		zBlob = (unsigned char *)malloc(nBlob);
		if (nBlob!=read(fd, zBlob, nBlob)) {
			perror("read");
			return 1;
		}
		close(fd);

		/* Write the blob to the database and free the memory allocated above. */
		if (SQLITE_OK!=writeBlob(db, zFile, zBlob, nBlob)) {
			databaseError(db);
			return 1;
		}
		free(zBlob);
	} else {
		/* A retrieve operation. Retrieve the blob from the database and
		** write it to the filesystem.
		*/
		int fd;
		int nBlob;
		unsigned char *zBlob;

		/* Open the file. Pass both O_CREAT and O_EXCL which means it is an
		** error if the file already exists. Safety first :)
		*/
		fd = open(zFile, O_WRONLY|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR);
		if (fd<0) {
			perror("open");
			return 1;
		}

		/* Read the blob from the database. If the entry does not exist print
		** out an error and exit.
		*/
		if (SQLITE_OK!=readBlob(db, zFile, &zBlob, &nBlob)) {
			databaseError(db);
			return 1;
		}
		if (!zBlob) {
			fprintf(stderr, "No such database entry: %s\n", zFile);
			return 1;
		}

		/* Write and close the file. */
		if (nBlob!=write(fd, zBlob, nBlob)) {
			perror("write");
			return 1;
		}
		close(fd);

		/* Free the blob read from the database */
		freeBlob(zBlob);
	}

	return 0;
}
#endif /* #ifndef OMIT_MAIN_FUNCTION */











