#ifndef	__ALRIGHT_SQL_H__
#define	__ALRIGHT_SQL_H__

#ifndef	NDEBUG
 #define DEBUG_PRINT(fmt,ARG...)      printf(fmt,##ARG)
#else
 #define DEBUG_PRINT(fmt,ARG...)
#endif
  
  
//  The max SQL command length.
#define SQL_MAX_CMD_LEN               256

//
//  Sql sytex format.
//
//  CREATE TABLE table_name (column_def[,column_def])
const char Sytex_CreateTable  []      =
"CREATE TABLE IF NOT EXISTS %s (%s)";

//  DROP TABLE table_name
const char Sytex_DropTable []         =
"DROP TABLE %s";

//	REFERENCES foreign_table (column_name)
const char Sytex_ForeignKey []        =
" REFERENCES %s (%s)";

//	INSERT INTO table_name (column_list) VALUES (value_list)
const char Sytex_InsertRecord []      =
"INSERT INTO %s (%s) VALUES (%s)";

//	DELETE FROM table_name [WHERE expr]
const char Sytex_DeleteRecord []      =
"DELETE FROM %s %s";

const char Sytex_SelectRecord []      =
"SELECT %s FROM %s %s";

//	SELECT sql FROM sqlite_master WHERE tbl_name = 'table_name' AND type = 'table'
const char Sytex_GetTableStatement [] =
"SELECT sql FROM sqlite_master WHERE tbl_name = '%s' AND type = 'table'";

//
//  Tag definition.
//
//  Data type
const char tagText []                 = "TEXT";
const char tagInteger []              = "INTEGER";
const char tagReal []                 = "REAL";
const char tagNone []                 = "NONE";

//	Constraint
const char tagPrimaryKey []           = "PRIMARY KEY";
const char tagAutoIncreament []       = "AUTOINCREMENT";
const char tagUnique []               = "UNIQUE";
const char tagNotNull []              = "NOT NULL";
const char tagDefault []              = "DEFAULT";
const char tagReferences []           = "REFERENCES";

//  Create table.(Parse for get statement)
const char tagCreateTable []          = "CREATE TABLE";

//	Condition Operator
//const char tagOperator[SQL_OP_END + 1][5]  = 
const char tagOperator[][5]           = 
{
  "OR",   //  SQL_OP_OR
  "AND",  //  SQL_OP_AND
  "==",   //  SQL_OP_EQUAL
  "!=",   //  SQL_OP_NOT_EQUAL
  "LIKE", //  SQL_OP_LIKE
  "<",    //  SQL_OP_LESS
  "<=",   //  SQL_OP_LESS_EQUAL
  ">",    //  SQL_OP_LARGE
  ">=",   //  SQL_OP_LARGE_EQUAL
  " "     //  SQL_OP_END
};

const char tagTransaction[][20]       = 
{
  "BEGIN TRANSACTION;",
  "ROLLBACK;",
  "COMMIT;"
};

typedef struct _APISQL
{
  SQL_T               SqlType;
  struct sqlite3      *hSqlEngine;
  char                szFilename[MAX_FILENAME_LEN];
  int                 SqliteRet;
}APISQL;

typedef struct _RECORD
{
  WORD                wColumn;
  WORD                wRecord;
  SQL_VALUE           **ppVal;    //  ppVal[wColumn][wRecord]
}RECORD;
#endif  //  __ALRIGHT_SQL_H__
