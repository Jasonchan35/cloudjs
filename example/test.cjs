cjs.print( '<head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/> </head>' );

function test() {
	cjs.print("===== cloud print test =========\n");
	cjs.dump_var( "http", http, 0 );	
}

cjs.print("<pre>");

test();

cjs.print("</pre>");
