var print = cjs.print;

//cjs.include( "some.cjs" );
cjs.includeOnce( "include_test.cjs" );
cjs.includeOnce( "include_test.cjs" );


print("my_print\n");
cjs.print( "testing 1234\n" );



//cjs.session['test'] = 123;
//cjs.session.test = 123;
//print("session = " + cjs.session.test + "\n" );

function test_sqlite() {
	cjs.module( "sqlite3" );
	//var db = SQLite3.open( ":memory:" );
	var db = SQLite3.open( "sqlite_test.db" );
	var stmt = db.exec( "drop table if exists table1" );
	var stmt = db.exec( "create table table1 ( num interger primary key, name varchar )" );
	for( var i=0; i<4; i++ ) {
		var stmt = db.exec( "insert into table1( name, num ) values(?,?) ", "abc", i*10 );
	}
	var stmt = db.exec( "select * from table1" );
	var row;
	while( row = stmt.getRow() ) {
		for( i in row ) {
			print( "\t"+row[i] );
		}
		print("\n");
	}
}

function test_pgsql() {
	cjs.module( "pgsql" );
	var db = PostgreSQL.connect( "host=127.0.0.1 dbname=testdb user=test password=1234" );
	var stmt = db.exec( "drop table if exists table1" );
	var stmt = db.exec( 'CREATE TABLE table1( num integer NOT NULL, "name" character varying, CONSTRAINT table1_pkey PRIMARY KEY (num) )' );
	for( var i=0; i<4; i++ ) {
		var stmt = db.exec( "insert into table1( name, num ) values(?,?) ", "abc", i*10 );
	}
	var stmt = db.exec( "select * from table1" );
	var row;
	while( row = stmt.getRow() ) {
		for( i in row ) {
			print( "\t"+row[i] );
		}
		print("\n");
	}
	
}

function test_mysql() {
	cjs.module( "mysql" );
	var db = MySQL.connect( "test", "test", "1234", "localhost" );
	var stmt = db.exec( "drop table if exists table1" );
	var stmt = db.exec( "CREATE TABLE table1( num int NULL , name varchar(255) NULL , PRIMARY KEY (num) );" );
	
	for( var i=0; i<4; i++ ) {
		var stmt = db.exec( "insert into table1( name, num ) values(?,?) ", "abc", i*10 );
	}
	var stmt = db.exec( "select * from table1" );
	var row;
	while( row = stmt.getRow() ) {
		for( i in row ) {
			print( "\t"+row[i] );
		}
		print("\n");
	}
	
}

function test_exec() {
	cjs.module( "exec" );
	var ret = cjs.exec("dir");

	print("exec status = " + ret.status + "\n" );	
	print("---- exec stdout ----\n" + ret.stdout + "\n" );	
	print("---- exec stderr ----\n" + ret.stderr + "\n" );	
	
}

//test_sqlite();
//test_pgsql();
//test_mysql();
//test_exec();

