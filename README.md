apisql
=================

A application interface to access SQL engine.

It is suppose to access various SQL engines which have it's own dynamic
library (.so). The functions in dynamic library would be ported to api_sql
automatic by function potinter like linux system call ioctrl().

#   Release  Note:
    #   api_sql-1.0.0
        #   Release Date: 2012-12-01
        #   Enhancements and Improvements:
            #   Implement basic function to access SQLite-3.7.14.1 database.
            #   Support multi primary key.
            #   Support foreign key cnotraint.
        #   Bugfix:
        #   Maintenance:
    #   api_sql-1.0.1
        #   Release Date: 2012-12-30
        #   Enhancements and Improvements:
        #   Bugfix:
            #   100001  : Sql syntax error while express vaule is string issue.
        #   Maintenance:
    #   api_sql-2.0.1.0
        #   Release Date: 2014-02-23
        #   Enhancements and Improvements:
        #   Bugfix:
        #   Maintenance:
            #   Change to support repository-2.x.x.
