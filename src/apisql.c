//  uci application interface for cli.

#include <unistd.h>
#include "apisql.h"
#include "sql.h"

//  SQL_RET_SUCCESS
//  SQL_RET_ERR_API
//  SQL_RET_ERR_UNSUPPORT
SQL_RET GetExpress(LPSTR pszExpr, SQL_EXPR *paExpr)
{
  SQL_RET         Ret = SQL_RET_SUCCESS;
  SQL_COLUMN_DEF  *pCol;
  int             nExprLen = 0;
  int             i;
  
  if (pszExpr && paExpr)
  {
    nExprLen += Co_Sprintf(pszExpr, nExprLen, "WHERE");
    i = -1;
    do
    {
      i++;
      pCol = paExpr[i].pCol;
      switch(pCol->Type)
      {
        case SQL_DATA_T_STRING:
          //  (+) 2012-12-30  Tag: 100001
          nExprLen += Co_Sprintf(pszExpr, nExprLen, " %s%s'%s'", 
          //  (-) 2012-12-30  Tag: 100001
            pCol->strName, tagOperator[paExpr[i].CondOp], 
            paExpr[i].Val.psz);
          break;
        case SQL_DATA_T_INTEGER:
          nExprLen += Co_Sprintf(pszExpr, nExprLen, " %s%s%d", 
            pCol->strName, tagOperator[paExpr[i].CondOp], 
            paExpr[i].Val.i);
          break;
        case SQL_DATA_T_DECIMAL:
          nExprLen += Co_Sprintf(pszExpr, nExprLen, " %s%s%f", 
            pCol->strName, tagOperator[paExpr[i].CondOp], 
            paExpr[i].Val.d);
          break;
        default:
          Ret = SQL_RET_ERR_UNSUPPORT;
          break;
      }  //  End switch()
      nExprLen += Co_Sprintf(pszExpr, nExprLen, " %s", 
        tagOperator[paExpr[i].AdjOp]);
    }while((paExpr[i].AdjOp != SQL_OP_END) && (Ret == SQL_RET_SUCCESS));
  }
  else
    Ret = SQL_RET_ERR_API;

  return Ret;

}

#ifdef  METHOD_EXEC
SQL_RET SqlExec(APISQL *pApi, LPSTR pszCmd, LPVOID pfnCallback, LPVOID pUserdatas)
{
  SQL_RET    Ret;
  int      SqliteRet;
  
  if (pApi && pszCmd && (strlen(pszCmd) < SQL_MAX_CMD_LEN))
  {
    printf ("SQL Cmd: %s\n", pszCmd);
    SqliteRet = sqlite3_exec(pApi->hSqlEngine, pszCmd, pfnCallback, pUserdatas, NULL);
    if (SqliteRet == SQLITE_OK)
      Ret = SQL_RET_SUCCESS;
    else
    {
      pApi->SqliteRet = SqliteRet;
      Ret = SQL_RET_ERR_SQLENGINE;
    }
  }
  else
    Ret = SQL_RET_ERR_COMMAND;
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}
#else
//  o/p:
//    SQL_RET_SUCCESS
//    SQL_RET_ERR_API
//    SQL_RET_ERR_SQLENGINE
//  Only for pRecord != NULL cae.
//    SQL_RET_ERR_SYSTEM
//    SQL_RET_ERR_UNSUPPORT
SQL_RET SqlExec(APISQL *pApi, LPSTR pszCmd, SQL_RECORD *pRecord)
{
  SQL_RET       Ret;
  int           SqliteRet;
  sqlite3_stmt  *pState;
  int           iCol;
  LPSTR         pszVal, psz;
  SQL_REC_COL   *pCol;
  
  //  Check command length.
  if ((strlen(pszCmd) > 0) && (strlen(pszCmd) < SQL_MAX_CMD_LEN))
    Ret = SQL_RET_SUCCESS;
  else
    Ret = SQL_RET_ERR_API;
  
  //  Prepare statement.
  if (Ret == SQL_RET_SUCCESS)
  {
    SqliteRet = 
      sqlite3_prepare(pApi->hSqlEngine, pszCmd, -1, &pState, NULL);
    if (SqliteRet != SQLITE_OK)
    {
      pApi->SqliteRet = SqliteRet;
      Ret = SQL_RET_ERR_SQLENGINE;
    }
  }

  if (Ret == SQL_RET_SUCCESS)
  {
    //  Get
    if (pRecord)
    {
      //  Get column number.
      pRecord->wColumn = sqlite3_column_count(pState);
  
      pRecord->paCol = 
        (SQL_REC_COL *) calloc (sizeof(SQL_REC_COL), pRecord->wColumn);
      if (pRecord->paCol)
      {
        pRecord->wRecord = 0;
        while (((SqliteRet = sqlite3_step(pState)) == SQLITE_ROW) && 
            (Ret == SQL_RET_SUCCESS))
        {
          for (iCol = 0; iCol < pRecord->wColumn; iCol++)
          {
            //  Get column info in first time sqlite3_step()
            if (pRecord->wRecord == 0)
            {
              //  Get column type.
              pRecord->paCol[iCol].Type = 
                sqlite3_column_type(pState, iCol);
              DEBUG_PRINT("Sqlite Type:%d\n", 
                sqlite3_column_type(pState, iCol));
                
              //  Get column name.
              pszVal = (LPSTR)sqlite3_column_name(pState, iCol);
              if (pszVal)
              {
                psz = (LPSTR) malloc (strlen(pszVal) + 1);
                if (psz)
                {
                  strcpy(psz, pszVal);
                  pRecord->paCol[iCol].pszColName = psz;
                }
                else
                {
                  Ret = SQL_RET_ERR_SYSTEM;
                  break;
                }
              }
              else
                pRecord->paCol[iCol].pszColName = NULL;
            }
            
            //  Get records.
            pCol = &pRecord->paCol[iCol];
            pCol->paVal = (SQL_VALUE *) realloc (pCol->paVal, 
              sizeof(SQL_VALUE) * (pRecord->wRecord + 1));
            if (pCol->paVal)
            {
              switch(pCol->Type)
              {
                case SQL_DATA_T_INTEGER:
                  pCol->paVal[pRecord->wRecord].i = 
                    sqlite3_column_int(pState, iCol);
                  
                  DEBUG_PRINT("pRecord->paCol[%d].paVal[%d].i:%d\n", 
                    iCol, pRecord->wRecord,
                    pCol->paVal[pRecord->wRecord].i);
                  break;
                case SQL_DATA_T_DECIMAL:
                  pCol->paVal[pRecord->wRecord].d = 
                    sqlite3_column_double(pState, iCol);
                  
                  DEBUG_PRINT("pRecord->paCol[%d].paVal[%d].i:%f\n", 
                    iCol, pRecord->wRecord,
                    pCol->paVal[pRecord->wRecord].d);
                  break;
                case SQL_DATA_T_STRING:
                  pszVal = (LPSTR)sqlite3_column_text(pState, iCol);
                  if (pszVal)
                  {
                    psz = (LPSTR) malloc (strlen(pszVal) + 1);
                    if (psz)
                      strcpy(psz, pszVal);
                    else
                    {
                      Ret = SQL_RET_ERR_SYSTEM;
                      break;
                    }
                    pCol->paVal[pRecord->wRecord].psz = psz;
                  }
                  else
                    pCol->paVal[pRecord->wRecord].psz = NULL;
                  
                  DEBUG_PRINT(
                    "pRecord->paCol[%d].paVal[%d].psz:%s\n", 
                    iCol, pRecord->wRecord,
                    pCol->paVal[pRecord->wRecord].psz);
                  break;
                case SQLITE_BLOB:
                  Ret = SQL_RET_ERR_UNSUPPORT;
//                    pCol->paVal[pRecord->wRecord].b = 
//                      sqlite3_column_blob(pState, iCol);
                  break;
                default:
                  Ret = SQL_RET_ERR_API;
                  break;
              }  //  End switch
            }
            else
            {
              Ret = SQL_RET_ERR_SYSTEM;
              break;
            }
          }  //  End for loop.
          pRecord->wRecord++;
        }  //  End while loop.
        if (SqliteRet != SQLITE_DONE)
        {
          pApi->SqliteRet = SqliteRet;
          Ret = SQL_RET_ERR_SQLENGINE;
        }
        
        //  Release pRecord here because pRecord->paCol is not NULL.
        //  It could avoid system crash due to free NULL pointer..
        if (Ret != SQL_RET_SUCCESS)
          ApiSql_ReleaseRecord(pRecord);
      }
      else
        Ret = SQL_RET_ERR_SYSTEM;
    }
    //  Set
    else
    {
      SqliteRet = sqlite3_step(pState);
      if (SqliteRet == SQLITE_DONE)
        Ret = SQL_RET_SUCCESS;
      else
      {
        pApi->SqliteRet = SqliteRet;
        Ret = SQL_RET_ERR_SQLENGINE;
      }
    }
    sqlite3_finalize(pState);
  }
  
  DEBUG_PRINT("[%s] %s() Ret=%d, Cmd=%s\n", __FILE__, __FUNCTION__, Ret, pszCmd);
  return Ret;
}
#endif  //  OLD_MATHOD

//    id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT,
//    ^                                            ^
//  pszStm                                       pszEnd
//  o/p:
//    SQL_RET_SUCCESS
//    SQL_RET_ERR_SYSTEM
SQL_RET GetColumnInfo(SQL_COLUMN_DEF *pCol, 
                      LPSTR pszStm, 
                      DWORD wStateLen)
{
  SQL_RET   Ret = SQL_RET_SUCCESS;
  LPSTR     pszStmEnd;
  LPSTR     psz, pszTemp, pszTemp2;
  
  //  Get column name.
  pszStmEnd = pszStm + wStateLen;
  pszTemp = Co_SkipCharacter(pszStm, SKIP_T_ALPHABET, TRUE);
  strncpy(pCol->strName, pszStm, pszTemp - pszStm);
  pCol->strName[pszTemp - pszStm] = 0;
  
  //  Find data type.
  if ((pszTemp = strstr(pszStm, tagText)) && (pszTemp < pszStmEnd))
    pCol->Type = SQL_DATA_T_STRING;
  else if ((pszTemp = strstr(pszStm, tagInteger)) && (pszTemp < pszStmEnd))
    pCol->Type = SQL_DATA_T_INTEGER;
  else if ((pszTemp = strstr(pszStm, tagReal)) && (pszTemp < pszStmEnd))
    pCol->Type = SQL_DATA_T_DECIMAL;
  else
    pCol->Type = SQL_DATA_T_END;
    
  //  Find the attribute.
  pCol->dwAttr = 0;
  
  //  UNIQUE
  if ((pszTemp = strstr(pszStm, tagUnique)) && (pszTemp < pszStmEnd))
    pCol->dwAttr += SQL_COL_ATTR_UNIQUE;
  
  //  NOT NULL
  if ((pszTemp = strstr(pszStm, tagNotNull)) && (pszTemp < pszStmEnd))
    pCol->dwAttr += SQL_COL_ATTR_NOTNULL;
  
  //  DEFAULT VALUE
  if ((pszTemp = strstr(pszStm, tagDefault)) && (pszTemp < pszStmEnd))
  {
    pCol->dwAttr += SQL_COL_ATTR_DEFAULT;
    pszTemp = Co_SkipCharacter(pszTemp + strlen(tagDefault), SKIP_T_SPACE, TRUE);
    switch(pCol->Type)
    {
      case SQL_DATA_T_STRING:
        pszTemp++;
        pszTemp2 = strchr(pszTemp, '"');
        pCol->DefVal.psz = (LPSTR) malloc (pszTemp2 - pszTemp + 1);
        if (pCol->DefVal.psz)
        {
          strncpy(pCol->DefVal.psz, pszTemp, pszTemp2 - pszTemp);
          pCol->DefVal.psz[pszTemp2 - pszTemp] = 0;
        }
        else
          Ret = SQL_RET_ERR_SYSTEM;
        break;
      case SQL_DATA_T_INTEGER:
        pCol->DefVal.i = atoi(pszTemp);
        break;
      case SQL_DATA_T_DECIMAL:
        pCol->DefVal.d = atof(pszTemp);
        break;
      default:
        pCol->DefVal.i = 0;
        break;
    }
  }
  
  //  PRIMARY KEY
  if ((pszTemp = strstr(pszStm, tagPrimaryKey)) && (pszTemp < pszStmEnd))
  {
    pCol->dwAttr += SQL_COL_ATTR_PRIMARYKEY;
    if ((pszTemp = strstr(pszTemp, tagAutoIncreament)) && (pszTemp < pszStmEnd))
      pCol->PrimaryKey.bAutoInc = TRUE;
    else
      pCol->PrimaryKey.bAutoInc = FALSE;
  }
  
  //  FOREIGN KEY
  if ((pszTemp = strstr(pszStm, tagReferences)) && (pszTemp < pszStmEnd))
  {
    pCol->dwAttr += SQL_COL_ATTR_FOREIGNKEY;
    
    //  Find the foreign table name.
    pszTemp = Co_SkipCharacter(pszTemp + strlen(tagReferences), SKIP_T_SPACE, TRUE);
    pszTemp2 = Co_SkipCharacter(pszTemp, SKIP_T_ALPHABET, TRUE);
    strncpy(pCol->ForeignKey.szForeTbl, pszTemp, pszTemp2 - pszTemp);
    pCol->ForeignKey.szForeTbl[pszTemp2 - pszTemp] = 0;
    
    //  Find the foreign column name.
    pszTemp = strchr(pszTemp, '(') + 1;
    if (pszTemp)
    {
      pszTemp = Co_SkipCharacter(pszTemp, SKIP_T_SPACE, TRUE);
      pszTemp2 = strchr(pszTemp, ')');
      pszTemp2 = Co_SkipCharacter(pszTemp2, SKIP_T_SPACE, FALSE);
      strncpy(pCol->ForeignKey.szColumn, pszTemp, pszTemp2 - pszTemp);
      pCol->ForeignKey.szColumn[pszTemp2 - pszTemp] = 0;
    }
  }
  
  return Ret;
}

SQL_HANDLE ApiSql_OpenDb(SQL_T Type, LPSTR pszDbPath)
{
  SQL_RET      Ret;
  int        SqliteRet;
  APISQL      *pApi = NULL;
  struct sqlite3  *hSqlEngine;
  LPSTR      psz;
  
  //  Check the input parameters.
  if (pszDbPath)
    Ret = SQL_RET_SUCCESS;
  else
    Ret = SQL_RET_ERR_PARAM;
  
  //  Open sqlite3 handle.
  if (Ret == SQL_RET_SUCCESS)
  {
    SqliteRet = sqlite3_open(pszDbPath, &hSqlEngine);
    if (SqliteRet != SQLITE_OK)
    {
      pApi->SqliteRet = SqliteRet;
      Ret = SQL_RET_ERR_SQLENGINE;
    }
  }
  
  //  Change sqlite3 config.
  if (Ret == SQL_RET_SUCCESS)
  {
    //  Enable foreign key contraints.
    SqliteRet = sqlite3_db_config(  hSqlEngine, 
                    SQLITE_DBCONFIG_ENABLE_FKEY, 
                    1, NULL);
    if (SqliteRet != SQLITE_OK)
    {
      pApi->SqliteRet = SqliteRet;
      Ret = SQL_RET_ERR_SQLENGINE;
    }
  }
  
  //  Initialize APISQL structure.
  if (Ret == SQL_RET_SUCCESS)
  {
    pApi = (APISQL *) malloc (sizeof(APISQL));
    if (pApi)
    {
      psz = strrchr(pszDbPath, '/');
      if (psz)
        strcpy(pApi->szFilename, ++psz);
      else
        strcpy(pApi->szFilename, pszDbPath);
      DEBUG_PRINT ("%s\n", pApi->szFilename);
      
      pApi->SqlType = Type;
      pApi->hSqlEngine = hSqlEngine;
    }
    else
    {
      SqliteRet = sqlite3_close(hSqlEngine);
      Ret = SQL_RET_ERR_SYSTEM;
    }
  }
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return (LPVOID)pApi;
}

SQL_RET ApiSql_CloseDb(SQL_HANDLE hSql)
{
  SQL_RET    Ret;
  int      SqliteRet;
  APISQL    *pApi = (APISQL *)hSql;
  
  //  Check the input parameter.
  if (pApi)
    Ret = SQL_RET_SUCCESS;
  else
    Ret = SQL_RET_ERR_PARAM;
  
  //  Close sqlite3 handle.
  if (Ret == SQL_RET_SUCCESS)
  {
    SqliteRet = sqlite3_close(pApi->hSqlEngine);
    if (SqliteRet != SQLITE_OK)
    {
      pApi->SqliteRet = SqliteRet;
      Ret = SQL_RET_ERR_SQLENGINE;
    }
  }
  
  //  Release structure APISQL
  if (Ret == SQL_RET_SUCCESS)
  {
    free(pApi);
    pApi = NULL;
  }
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}

SQL_RET ApiSql_CreateTable(SQL_HANDLE hSql, SQL_TABLE_DEF *pTable)
{
  SQL_RET         Ret;
  APISQL          *pApi = (APISQL *)hSql;
  SQL_COLUMN_DEF  *paColdef;
  SQL_STRING      strCmd, strColdef;
  int             nLen = 0;
  int             i;
  
  //  Check the input parameters.
  if (pApi && pTable)
  {
    if (pTable->wColumn > 0)
    {
      Ret = SQL_RET_SUCCESS;
      for (i = 0; i < pTable->wColumn; i++)
      {
        if (&pTable->paColDef[i] == NULL)
        {
          Ret = SQL_RET_ERR_PARAM;
          break;
        }
      }
    }
    else
      Ret = SQL_RET_ERR_PARAM;
  }
  
  //  Create table.
  if (Ret == SQL_RET_SUCCESS)
  {
    //  Prepare column_def
    paColdef = pTable->paColDef;
    for (i = 0; (i < pTable->wColumn) && (Ret == SQL_RET_SUCCESS); i++)
    {
      if (i > 0)
        nLen += Co_Sprintf(strColdef, nLen, ", ");
      
      //  Data type and default value
      switch(paColdef[i].Type)
      {
        case SQL_DATA_T_STRING:
        case SQL_DATA_T_DATA:
        case SQL_DATA_T_TIME:
        case SQL_DATA_T_DATATIME:
        {
          if (paColdef[i].dwAttr & SQL_COL_ATTR_DEFAULT)
          {
            nLen += Co_Sprintf(strColdef, nLen, "%s %s %s \"%s\"", 
              paColdef[i].strName, tagText, 
              tagDefault, paColdef[i].DefVal.psz);
          }
          else
          {
            nLen += Co_Sprintf(strColdef, nLen, "%s %s", 
              paColdef[i].strName, tagText);
          }
          break;
        }
        case SQL_DATA_T_INTEGER:
        case SQL_DATA_T_BOOL:
        {
          if (paColdef[i].dwAttr & SQL_COL_ATTR_DEFAULT)
          {
            nLen += Co_Sprintf(strColdef, nLen, "%s %s %s %d", 
              paColdef[i].strName, tagInteger,
              tagDefault, paColdef[i].DefVal.i);
          }
          else
          {
            nLen += Co_Sprintf(strColdef, nLen, "%s %s", 
              paColdef[i].strName, tagInteger);
          }
          break;
        }
        case SQL_DATA_T_DECIMAL:
        {
          if (paColdef[i].dwAttr & SQL_COL_ATTR_DEFAULT)
          {
            nLen += Co_Sprintf(strColdef, nLen, "%s %s %s %f", 
              paColdef[i].strName, tagReal, tagDefault, paColdef[i].DefVal.d);
          }
          else
          {
            nLen += Co_Sprintf(strColdef, nLen, "%s %s", 
              paColdef[i].strName, tagReal);
          }
          break;
        }
        default:
          Ret = SQL_RET_ERR_PARAM;
          break;
      }  //  End switch()
      
      //  Unique
      if (paColdef[i].dwAttr & SQL_COL_ATTR_UNIQUE)
        nLen += Co_Sprintf(strColdef, nLen, " %s", tagUnique);
      
      //  Not null
      if (paColdef[i].dwAttr & SQL_COL_ATTR_NOTNULL)
        nLen += Co_Sprintf(strColdef, nLen, " %s", tagNotNull);
      
      //  Primary key
      if (paColdef[i].dwAttr & SQL_COL_ATTR_PRIMARYKEY)
      {
        if (paColdef[i].PrimaryKey.bAutoInc == TRUE)
          nLen += Co_Sprintf(strColdef, nLen, " %s %s", 
            tagPrimaryKey, tagAutoIncreament);
        else
          nLen += Co_Sprintf(strColdef, nLen, " %s", tagPrimaryKey);
      }
      
      //  Foreign key
      if (paColdef[i].dwAttr & SQL_COL_ATTR_FOREIGNKEY)
      {
        if ((paColdef[i].ForeignKey.szForeTbl[0] != 0) && 
          (paColdef[i].ForeignKey.szColumn[0] != 0))
        {
          nLen += Co_Sprintf(strColdef, nLen, (LPSTR)Sytex_ForeignKey, 
            paColdef[i].ForeignKey.szForeTbl, 
            paColdef[i].ForeignKey.szColumn);
        }
        else
        {
          Ret = SQL_RET_ERR_PARAM;
          break;
        }
      }
    }  //  End for loop.
    
    if (Ret == SQL_RET_SUCCESS)
    {
      Co_Sprintf(strCmd, 0, (LPSTR)Sytex_CreateTable, pTable->strName, strColdef);
      Ret = SqlExec(pApi, strCmd, NULL);
    }
  }
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}

SQL_RET ApiSql_DropTable(SQL_HANDLE hSql, LPSTR pszTable)
{
  SQL_RET      Ret;
  APISQL      *pApi = (APISQL *)hSql;
  SQL_STRING  strCmd;
  
  //  Check the input parameters.
  if (pApi && pszTable)
    Ret = SQL_RET_SUCCESS;
  else
    Ret = SQL_RET_ERR_PARAM;
  
  if (Ret == SQL_RET_SUCCESS)
  {
    Co_Sprintf(strCmd, 0, (LPSTR)Sytex_DropTable, pszTable);
    Ret = SqlExec(pApi, strCmd, NULL);
  }
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}

SQL_RET ApiSql_Select(SQL_HANDLE      hSql, 
                      SQL_TABLE_DEF   *pTable, 
                      WORD            wColumn,
                      SQL_COLUMN_DEF  **ppCol, 
                      SQL_EXPR        *paExpr, 
                      SQL_RECORD      *pRecord)
{
  SQL_RET     Ret;
  APISQL      *pApi = (APISQL *)hSql;
  SQL_STRING  strCmd, strCol, strExpr;
  int         nColLen = 0;
  int         i;
  
  //  Check the input parameters.
  if (pApi && pTable && pRecord)
  {
    if (wColumn)
    {
      //  Check all ppCol are not NULL.
      if (wColumn < pTable->wColumn)
      {
        for (i = 0; i < wColumn; i++)
        {
          if (ppCol[i] == NULL)
          {
            Ret = SQL_RET_ERR_PARAM;
            break;
          }
        }
      }
      else
        Ret = SQL_RET_ERR_PARAM;
      
      //  Check all column definition are exist in pTable
      if ((Ret == SQL_RET_SUCCESS) && (pTable->wColumn > 0))
      {
        Ret = SQL_RET_SUCCESS;
        for (i = 0; i < pTable->wColumn; i++)
        {
          if (&pTable->paColDef[i] == NULL)
          {
            Ret = SQL_RET_ERR_PARAM;
            break;
          }
        }
      }
      else
        Ret = SQL_RET_ERR_PARAM;
    }
    else
      Ret = SQL_RET_SUCCESS;
  }
  else
    Ret = SQL_RET_ERR_PARAM;
  
  //  Prepare sql command.
  if (Ret == SQL_RET_SUCCESS)
  {
    //  Prepare column string.
    if (wColumn)
    {
      for (i = 0; i < wColumn; i++)
      {
        if (i > 0)
          nColLen += Co_Sprintf(strCol, nColLen, ",%s", ppCol[i]->strName);
        else
          nColLen += Co_Sprintf(strCol, nColLen, "%s", ppCol[i]->strName);
      }
    }
    else
      Co_Sprintf(strCol, nColLen, "*");
    
    //  Prepare express string.
    if (paExpr)
      Ret = GetExpress(strExpr, paExpr);
    else
      strExpr[0] = 0;

    //  Prepare final command and execute.
    if (Ret == SQL_RET_SUCCESS)
    {
      Co_Sprintf(strCmd, 0, (LPSTR)Sytex_SelectRecord, 
        strCol, pTable->strName, strExpr);
      
      Ret = SqlExec(pApi, strCmd, pRecord);
    }
  }
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}

SQL_RET ApiSql_GetTableDefine(SQL_HANDLE hSql, 
                              LPSTR pszTable,
                              SQL_TABLE_DEF *pTableDef)
{
  SQL_RET     Ret;
  APISQL      *pApi = (APISQL *)hSql;
  SQL_STRING  strCmd;
  SQL_RECORD  Record;
  LPSTR       psz, pszTemp, pszEnd;
  int         iRec;
  
  //  Check the input parameters.
  if (pApi && pszTable && pTableDef)
    Ret = SQL_RET_SUCCESS;
  else
    Ret = SQL_RET_ERR_PARAM;
  
  //  Get create statements
  if (Ret == SQL_RET_SUCCESS)
  {
    Co_Sprintf(strCmd, 0, (LPSTR)Sytex_GetTableStatement, pszTable);
    Ret = SqlExec(pApi, strCmd, &Record);
  }
  
  //  Get the table define.
  if (Ret == SQL_RET_SUCCESS)
  {
    //  Find the record that match the input table name.
    //  And point the begin locatino of the column define statement in psz.
    for (iRec = 0; iRec < Record.wRecord; iRec++)
    {
      psz = strchr(Record.paCol[iRec].paVal[0].psz, (int)'(');
      pszTemp = strstr (Record.paCol[iRec].paVal[0].psz, pszTable);
      if ((pszTemp) && (psz) && (pszTemp < psz))
      {
        strcpy(pTableDef->strName, pszTable);
        Ret = SQL_RET_SUCCESS;
        break;
      }
      else
        Ret = SQL_RET_ERR_NOTFOUND;
    }
    
    //  Get the column define statement for each column for GetColumnInfo()
    //  to get the column info.
    if (Ret == SQL_RET_SUCCESS)
    {
      pTableDef->wColumn = 0;
      pTableDef->paColDef = NULL;
      while(psz)
      {
        psz = Co_SkipCharacter(psz + 1, SKIP_T_SPACE, TRUE);
        pszEnd = strchr(psz, ',');
        if (pszEnd == NULL)
          pszEnd = strrchr(psz, ')');
        
        //  Dynamic allocate memory for paColDef.
        pTableDef->paColDef = 
          (SQL_COLUMN_DEF *) realloc (pTableDef->paColDef, 
          sizeof(SQL_COLUMN_DEF) * (pTableDef->wColumn + 1));
        if (pTableDef->paColDef)
        {
          Ret = GetColumnInfo(&pTableDef->paColDef[pTableDef->wColumn], 
            psz, pszEnd - psz);
        }
        else
        {
          Ret = SQL_RET_ERR_SYSTEM;
          break;
        }
          
        pTableDef->wColumn++;
        psz = strchr(psz + 1, ',');
      }
    }
    
    ApiSql_ReleaseRecord(&Record);
  }
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}

SQL_RET ApiSql_Transaction(SQL_HANDLE hSql, SQL_TRANS_CMD Cmd)
{
  SQL_RET     Ret;
  APISQL      *pApi = (APISQL *)hSql;
  SQL_STRING  strCmd;
  
  //  Check the input parameters.
  if ((pApi) && (Cmd >= SQL_TRANS_CMD_START) && (Cmd <= SQL_TRANS_CMD_COMMIT))
    Ret = SQL_RET_SUCCESS;
  else
    Ret = SQL_RET_ERR_PARAM;

  if (Ret == SQL_RET_SUCCESS)
    Ret = SqlExec(pApi, tagTransaction[Cmd], NULL);
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}

SQL_RET ApiSql_ReleaseRecord(SQL_RECORD *pRecord)
{
  SQL_RET      Ret = SQL_RET_SUCCESS;
  int          iRec, iCol;
  SQL_REC_COL  *pCol;
  
  if (pRecord)
  {
    if (pRecord->paCol)
    {
      for (iCol = 0; iCol < pRecord->wColumn; iCol++)
      {
        //  Release string buffer in each record.
        pCol = &pRecord->paCol[iCol];
        if (pCol->paVal)
        {
          if (pCol->Type == SQL_DATA_T_STRING)
          {
            for (iRec = 0; iRec < pRecord->wRecord; iRec++)
            {
              if (pCol->paVal[iRec].psz)
              {
                free(pCol->paVal[iRec].psz);
                pCol->paVal[iRec].psz = NULL;
              }
            }
          }
          
          //  Release each SQL_VALUE in column
          free (pCol->paVal);
          pCol->paVal = NULL;
        }
        
        //  Release column name buffer in each column
        if (pCol->pszColName)
        {
          free (pCol->pszColName);
          pCol->pszColName = NULL;
        }
      }

      //  Release SQL_REC_COL
      free (pRecord->paCol);
      pRecord->paCol = NULL;
    }
  }
  else
    Ret = SQL_RET_ERR_PARAM;
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}

int ApiSql_GetSqlErrCode(SQL_HANDLE hSql)
{
  APISQL    *pApi;
  
  if (hSql)
  {
    pApi = (APISQL *)hSql;
    return pApi->SqliteRet;
  }
  else
    return SQL_RET_ERR_PARAM;
}


SQL_RET ApiSql_Insert(SQL_HANDLE      hSql, 
                      SQL_TABLE_DEF   *pTable, 
                      WORD            wColumn,
                      SQL_COLUMN_DEF  **ppCol, 
                      SQL_VALUE       *paVal)
{
  SQL_RET     Ret;
  APISQL      *pApi = (APISQL *)hSql;
  SQL_STRING  strCmd, strColList, strValList;
  int         nColLen = 0, nValLen = 0;
  int         i;

  //  Check the input parameters.
  if (pApi && pTable)
  {
    if (pTable->wColumn >= wColumn)
    {
      Ret = SQL_RET_SUCCESS;
      for (i = 0; i < wColumn; i++)
      {
        if (ppCol[i] == NULL)
        {
          Ret = SQL_RET_ERR_PARAM;
          break;
        }
      }
    }
    else
      Ret = SQL_RET_ERR_PARAM;
  }
  else
    Ret = SQL_RET_ERR_PARAM;
  
  //  Prepare column list and value list.
  if (Ret == SQL_RET_SUCCESS)
  {
    for (i = 0; (i < wColumn) && (Ret == SQL_RET_SUCCESS); i++)
    {
      nColLen += Co_Sprintf(strColList, nColLen, nColLen == 0 ? "%s" : ", %s", 
        ppCol[i]->strName);
      switch (ppCol[i]->Type)
      {
        case SQL_DATA_T_INTEGER:
          nValLen += Co_Sprintf(strValList, nValLen, 
            nValLen == 0 ? "%d" : ", %d", paVal[i].i);
          break;
        case SQL_DATA_T_DECIMAL:
          nValLen += Co_Sprintf(strValList, nValLen, 
            nValLen == 0 ? "%f" : ", %f", paVal[i].d);
          break;
        case SQL_DATA_T_STRING:      
          nValLen += Co_Sprintf(strValList, nValLen, 
            nValLen == 0 ? "'%s'" : ", '%s'", paVal[i].psz);
          break;
        default:
          Ret = SQL_RET_ERR_UNSUPPORT;
      }//  End switch()
    }  //  End for loop
  }
  
  //  Prepare final command and execute.
  if (Ret == SQL_RET_SUCCESS)
  {
    Co_Sprintf(strCmd, 0, (LPSTR)Sytex_InsertRecord, 
      pTable->strName, strColList, strValList);
    Ret = SqlExec(pApi, strCmd, NULL);
  }

  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}

SQL_RET ApiSql_Delete(SQL_HANDLE    hSql,
                      SQL_TABLE_DEF *pTable,
                      SQL_EXPR      *paExpr)
{
  SQL_RET      Ret;
  APISQL      *pApi = (APISQL *)hSql;
  SQL_STRING  strCmd, strExpr;
  
  //  Check the input parameters.
  if (pApi && pTable)
    Ret = SQL_RET_SUCCESS;
  else
    Ret = SQL_RET_ERR_PARAM;
  
  //  Prepare sql command.
  if (Ret == SQL_RET_SUCCESS)
  {
    if (paExpr)
    {
      Ret = GetExpress(strExpr, paExpr);
      if (Ret == SQL_RET_SUCCESS)
      {
        Co_Sprintf(strCmd, 0, (LPSTR)Sytex_DeleteRecord, 
          pTable->strName, strExpr);
      }
    }
    else
    {  
      Co_Sprintf(strCmd, 0, (LPSTR)Sytex_DeleteRecord, 
        pTable->strName, "");
    }
  }
  
  //  Execute sql command.
  if (Ret == SQL_RET_SUCCESS)
    Ret = SqlExec(pApi, strCmd, NULL);
  
  DEBUG_PRINT("[%s] %s() Ret = %d\n", __FILE__, __FUNCTION__, Ret);
  return Ret;
}
