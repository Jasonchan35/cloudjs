cjs.include("test.cjs");
cjs.module( "sqlite3" );
cjs.print( "testing 1234\n" );

var db = SQLite3.open( ":memory:" );
//var db = SQLite3.open( "sqlite3_test.db" );


var stmt = db.exec( "drop table if exists table1" );

var stmt = db.exec( "create table table1 ( num interger primary key, name varchar )" );

for( var i=0; i<20; i++ ) {
	var stmt = db.exec( "insert into table1( name, num ) values(?,?) ", "abc", i );
}

var stmt = db.exec( "select * from table1" );

cjs.print('<table border=1 width="200">');

while( 1 ) {
	var row = stmt.getRow();
	if( ! row ) break;

	print("<tr>");
	for( i in row ) {
		cjs.print( "<td>"+row[i]+"</td>" );
	}
	print("</tr>");
}

print("</table>");
