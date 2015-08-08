//  api_sql - A application interface to access SQL engine.
//  It is suppose to access various SQL engines which have it's own dynamic
//  library (.so). The functions in dynamic library would be ported to api_sql
//  automatic by function potinter like linux system call ioctrl().
//
//
//  Release  Note:
//    api_sql-1.0.0  :
//      Author : Albert Huang
//      Release Date: 2012-12-01
//      Enhancements and Improvements:
//        # Implement basic function to access SQLite-3.7.14.1 database.
//        # Support multi primary key.
//        # Support foreign key cnotraint.
//      Bugfix:
//      Maintenance:
//    api_sql-1.0.1  :
//      Author : Albert Huang
//      Release Date: 2012-12-30
//      Enhancements and Improvements:
//      Bugfix:
//        # 100001  : Sql syntax error while express vaule is string issue.
//      Maintenance:

#ifndef  __ALRIGHT_API_SQL_H__
#define  __ALRIGHT_API_SQL_H__

#define  API_SQL_VERSION      "1.0.0"

//#include "common.h"
#include "libcommon.h"

//  The SQL engine type enumulation.
typedef enum
{
  SQL_T_START,
  SQL_T_SQLITE,
  SQL_T_END
}SQL_T;

#include "sqlite3.h"

//  Define value structure
//  The SQL_VALUE is use between upper layer function and api_sql.
typedef union _SQL_VALUE
{
  LPSTR                     psz;
  int                       i;
  double                    d;
  BOOL                      b;
}SQL_VALUE;

#define  MAX_FILENAME_LEN   255
#define  SQL_MAX_STR_LEN    4096
typedef char                SQL_STRING[SQL_MAX_STR_LEN], *PSQL_STRING;

//  Define the max length of name, such like table name, column name...etc.
#define  SQL_MAX_NAME_LEN   64
typedef char                SQL_STR_NAME[SQL_MAX_NAME_LEN];

typedef enum
{
  SQL_RET_ERR_SYSTEM = -100,
  SQL_RET_ERR_API,
  SQL_RET_ERR_SQLENGINE,
  SQL_RET_ERR_COMMAND,
  SQL_RET_ERR_UNSUPPORT,
  SQL_RET_ERR_NOTFOUND,
  SQL_RET_ERR_PARAM,
  SQL_RET_SUCCESS = 0,
  SQL_RET_END
}SQL_RET;

//
//  Data stuff
//
//  Data type definition. 
//  These types are defined for api_sql not completely match the SQL engine.
typedef enum
{
  SQL_DATA_T_INTEGER = SQLITE_INTEGER,  //  1
  SQL_DATA_T_DECIMAL = SQLITE_FLOAT,    //  2
  SQL_DATA_T_STRING = SQLITE3_TEXT,     //  3
  SQL_DATA_T_BINARY = SQLITE_BLOB,      //  4 Unsupport.
  SQL_DATA_T_BOOL,
  SQL_DATA_T_DATA,      //  It's format is YYYY-MM-DD
  SQL_DATA_T_TIME,      //  It's format is HH:MM:SS
  SQL_DATA_T_DATATIME,    //  It's format is YYYY-MM-DD HH:MM:SS
  SQL_DATA_T_END
}SQL_DATA_T;

typedef struct _SQL_REC_COL
{
  LPSTR                     pszColName;
  SQL_DATA_T                Type;
  SQL_VALUE                 *paVal;
}SQL_REC_COL;

typedef struct _SQL_RECORD
{
  WORD                      wRecord;
  WORD                      wColumn;
  SQL_REC_COL               *paCol;
}SQL_RECORD;

//
//  Table stuff
//
//  Column definition.
typedef struct _SQL_COLUMN_DEF
{
  SQL_STR_NAME              strName;
  SQL_DATA_T                Type;
  DWORD                     dwAttr;      //  SQL_COL_ATTR_
  SQL_VALUE                 DefVal;
  //  Primary key attribute.
  //  Only reference in case SQL_COL_ATTR_PRIMARYKEY is selected.
  struct
  {
    BOOL                    bAutoInc;
  }PrimaryKey;
  //  Foreign key
  //  Only reference in case SQL_COL_ATTR_FOREIGNKEY is selected.
  struct
  {
    char                    szForeTbl[SQL_MAX_NAME_LEN];
    char                    szColumn[SQL_MAX_NAME_LEN];
  }ForeignKey;
}SQL_COLUMN_DEF;
                              
//  Table definition.
//  All columns defition are stored in a array which header pointer is
//  paColDef and the array number must equal to wColumn.
typedef struct _SQL_TABLE_DEF
{
  SQL_STR_NAME              strName;
  WORD                      wColumn;
  SQL_COLUMN_DEF            *paColDef;
}SQL_TABLE_DEF;

//  Column attribute definition.
#define  SQL_COL_ATTR_UNIQUE      0x00000001UL
#define  SQL_COL_ATTR_NOTNULL     0x00000002UL
#define  SQL_COL_ATTR_DEFAULT     0x00000004UL
#define  SQL_COL_ATTR_PRIMARYKEY  0x00000008UL
#define  SQL_COL_ATTR_FOREIGNKEY  0x00000010UL

//
//  Condition stuff
//
typedef enum
{
  SQL_OP_OR = 0,       //  OR
  SQL_OP_AND,          //  AND
  SQL_OP_EQUAL,        //  ==
  SQL_OP_NOT_EQUAL,    //  !=
  SQL_OP_LIKE,         //  LIKE
  SQL_OP_LESS,         //  <
  SQL_OP_LESS_EQUAL,   //  <=
  SQL_OP_LARGE,        //  >
  SQL_OP_LARGE_EQUAL,  //  >=
  SQL_OP_END
}SQL_OP;

//  The Expression is  (col_A > 5) and (col_B < 10) or (col_C == 2)
//  Condition Index   :  0              1               2
//  pszCol            :  colname_A      colname_B       colname_C
//  CondOp            :  SQL_OP_LARGE   SQL_OP_LESS     SQL_OP_EQUAL
//  Val               :  5              10              2
//  AdjOp             :  SQL_OP_AND     SQL_OP_OR       SQL_OP_END
typedef struct _SQL_EXPR
{
  SQL_COLUMN_DEF            *pCol;
  SQL_OP                    CondOp;
  SQL_VALUE                 Val;
  SQL_OP                    AdjOp;
}SQL_EXPR;

typedef enum
{
  SQL_TRANS_CMD_START = 0,
  SQL_TRANS_CMD_REVERT,
  SQL_TRANS_CMD_COMMIT
}SQL_TRANS_CMD;

typedef struct APISQL*      SQL_HANDLE;

//  Describe:
//    Open a API handle with designated DB file.
//    If the DB file not exist, it will create a empty one.
//  i/p:
//    Type      : The SQL engine type.
//    pszDbPath : The database file path.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_PARAM     : Input parameters error.
//    SQL_RET_ERR_SYSTEM    : Allocate memory failure.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
SQL_HANDLE ApiSql_OpenDb(SQL_T Type, LPSTR pszDbPath);

//  Describe:
//    Close the API handle.
//  i/p:
//    hSql : The API handle.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_PARAM     : Input parameters error.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
SQL_RET ApiSql_CloseDb(SQL_HANDLE hSql);

//  i/p:
//    hSql    : The API handle.
//    pTable  : Table definition.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_PARAM     : Input parameters error.
//    SQL_RET_ERR_API       : Api program internal error.
//    SQL_RET_ERR_SYSTEM    : Memory allocate failure.
//    SQL_RET_ERR_UNSUPPORT : SQL data type unsupport.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
SQL_RET ApiSql_CreateTable(SQL_HANDLE hSql, SQL_TABLE_DEF *pTable);

//  i/p:
//    hSql      : The API handle.
//    pszTable  : The table name.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_PARAM     : Input parameters error.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
SQL_RET ApiSql_DropTable(SQL_HANDLE hSql, LPSTR pszTable);

//  Describe:
//    Insert data into table.
//  i/p:
//    hSql    : The API handle.
//    pTable  : Table definition.
//    wColumn : Inserted column number.
//    ppCol   : Inserted column definitions. The number of ppaVal mest the same 
//              as wColumnThe column which not in the ppaCol would be insert as 
//              the default value if the default value be set in create table 
//              statement or be NULL value if the default value were not be set 
//              and the column could be NULL value.
//    ppVal   : Inserted values. The number of ppaVal mest the same as wColumn and 
//              the order in ppaVal must match with the order of ppaCol either.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_PARAM     : Input parameters error.
//    SQL_RET_ERR_API       : Api program internal error.
//    SQL_RET_ERR_UNSUPPORT : SQL data type unsupport.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
SQL_RET ApiSql_Insert(SQL_HANDLE      hSql, 
                      SQL_TABLE_DEF   *pTable, 
                      WORD            wColumn,
                      SQL_COLUMN_DEF  **ppCol, 
                      SQL_VALUE       *paVal);

//  Describe:
//    Delete data from table.
//  i/p:
//    hSql    : The API handle.
//    pTable  : Table definition.
//    paExpr  : The expression of delete statement. If paExpr set to NULL it
//              would delete all records from the table.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_PARAM     : Input parameters error.
//    SQL_RET_ERR_API       : Api program internal error.
//    SQL_RET_ERR_UNSUPPORT : SQL data type unsupport.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
SQL_RET ApiSql_Delete(SQL_HANDLE    hSql,
                      SQL_TABLE_DEF *pTable,
                      SQL_EXPR      *paExpr);

//  i/p:
//    hSql    : The API handle.
//    pTable  : Selected table.
//    wColumn : Selected column number. If wColumn is 0 it would select all 
//              column in the order of the create statement.
//    ppCol   : Selected column definitions. The records are selected in the 
//              order with the order in paCol. 
//    paExpr  : The expression of select statement. If paExpr set to NULL it
//              would select all records without any expression.
//    pRecord : <OUTPUT> The selected result.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_PARAM     : Input parameters error.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
//    SQL_RET_ERR_API       : Api program internal error.
//    SQL_RET_ERR_UNSUPPORT : SQL data type unsupport.
SQL_RET ApiSql_Select(SQL_HANDLE      hSql, 
                      SQL_TABLE_DEF   *pTable, 
                      WORD            wColumn,
                      SQL_COLUMN_DEF  **ppCol, 
                      SQL_EXPR        *paExpr, 
                      SQL_RECORD      *pRecord);

//  Describe:
//    Get the table defintion from the create table statement.
//  i/p:
//    hSql      : The API handle.
//    pszTable  : The table name.
//    pTableDef : <OUTPUT> The table definition.
//                The program would allocate memory for each column to load
//                its definition information, these memory should be released.
//                If the column's default value has been set and the data type 
//                is SQL_DATA_T_STRING, the string buffer in SQL_VALUE
//                (pColDef->DefVal.psz) need to be released either.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_PARAM     : Input parameters error.
//    SQL_RET_ERR_API       : Api program internal error.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
//    SQL_RET_ERR_NOTFOUND  : The column not fund in the table.
//    SQL_RET_ERR_SYSTEM    : Allocate memory failure.
SQL_RET ApiSql_GetTableDefine(SQL_HANDLE hSql, 
                              LPSTR pszTable,
                              SQL_TABLE_DEF *pTableDef);

//  Describe:
//    The commit/revert mechanism.
//    The mechanism start after the SQL_TRANS_CMD_START command. and stop at 
//    the SQL_TRANS_CMD_REVERT or SQL_TRANS_CMD_COMMIT command.
//    SQL_TRANS_CMD_COMMIT command would save the change into database.
//    SQL_TRANS_CMD_REVERT command would feedback the state before 
//    SQL_TRANS_CMD_START command.
//  i/p:
//    hSql : The API handle.
//    Cmd  : The command.
//  o/p:
//    SQL_RET_SUCCESS       : Success.
//    SQL_RET_ERR_API       : Api program internal error.
//    SQL_RET_ERR_SQLENGINE : SQL engine error. ApiSql_GetSqlErrCode() 
//                            could get the last error code.
SQL_RET ApiSql_Transaction(SQL_HANDLE hSql, SQL_TRANS_CMD Cmd);

//  Describe:
//    Release the all pRecord memory include column name buffer and string
//    buffer in each record.
//  i/p:
//    pRecord  : The SQL_RECORD pointer.
//  o/p:
//    SQL_RET_SUCCESS      : Success.
//    SQL_RET_ERR_PARAM    : Input parameters error.
SQL_RET ApiSql_ReleaseRecord(SQL_RECORD *pRecord);

//  Describe:
//    Get the last error code generated from sql engine.
//  i/p:
//    hSql  : The API handle.
//  o/p:
//    Sql engin error code.
int ApiSql_GetSqlErrCode(SQL_HANDLE hSql);
#endif  //  __ALRIGHT_API_SQL_H__