//cloud.include( "some.cjs" );
cloud.includeOnce( "include_test.cjs" );
cloud.includeOnce( "include_test.cjs" );

print("my_print\n");
cloud.print( "testing 1234\n" );

cloud.session['test'] = 123;

cloud.session.test = 123;

print("session = " + cloud.session.test + "\n" );

function test_sqlite() {
	cloud.module( "sqlite3" );
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
	cloud.module( "pgsql" );
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
	cloud.module( "mysql" );
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
	cloud.module( "exec" );
	var ret = cloud.exec("dir");

	print("exec status = " + ret.status + "\n" );	
	print("---- exec stdout ----\n" + ret.stdout + "\n" );	
	print("---- exec stderr ----\n" + ret.stderr + "\n" );	
	
}

//test_sqlite();
//test_pgsql();
//test_mysql();
//test_exec();

