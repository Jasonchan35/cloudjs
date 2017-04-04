![logo](art/logo/cloudjs_logo.png)

-----------------
Running Javascript on Server-side with Database support

## How it works:
When web server got the http request with file extension .cjs
will running backend javascript to handle response

## Supported Web Server:
* Apache 2.x

## Feature:
* Session variable
* modules
* timeout limit
* Shell execute
* Database access

## current modules:
* cjs_mod_exec     - exec shell command
* cjs_mod_database - common interface for database
* cjs_mod_sqlite3  - sqlite
* cjs_mod_mysql    - mysql
* cjs_mod_pgsql    - postgreSQL


### Example:
Http Request
```
http://.../test.cjs
```

### test.cjs
```javascript
cjs.include("test.cjs");
cjs.module( "sqlite3" );
cjs.print( "testing 1234\n" );

var db = SQLite3.open( "sqlite3_test.db" );
var stmt = db.exec( "drop table if exists table1" );
var stmt = db.exec( "create table table1 ( num interger primary key, name varchar )" );

for( var i=0; i<20; i++ ) {
	var stmt = db.exec( "insert into table1( name, num ) values(?,?) ", "abc", i );
}

var stmt = db.exec( "select * from table1" );
cjs.print('<table border=1 width="200">');

while(true) {
	var row = stmt.getRow();
	if( ! row ) break;

	print("<tr>");
	for( i in row ) {
		cjs.print( "<td>"+row[i]+"</td>" );
	}
	print("</tr>");
}

print("</table>");
```

