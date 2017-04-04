cloud.includeOnce("test.cjs");
cloud.includeOnce("test.cjs");
cloud.includeOnce("test.cjs");
cloud.includeOnce("test.cjs");

cloud.module( "sqlite3" );

print( "testing 1234\n" );

//var db = SQLite3.open( ":memory:" );
var db = SQLite3.open( "sqlite3_test.db" );


var stmt = db.exec( "drop table if exists table1" );

var stmt = db.exec( "create table table1 ( num interger primary key, name varchar )" );

for( var i=0; i<4; i++ ) {
	var stmt = db.exec( "insert into table1( name, num ) values(?,?) ", "abc", i*10 );
}

var stmt = db.exec( "select * from table1" );


cloud.print("<table border=1>");

var row;
while( row = stmt.getRow() ) {
	print("<tr>");
	for( i in row ) {
		cloud.print( "<td>"+row[i]+"</td>" );
	}
	print("</tr>");
}

print("</table>");
