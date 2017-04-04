

print( '<head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/> </head>' );


function print_indent( n ) {
	var s ="";
	for( i=0; i<n; i++ ) {
		s += "    ";
	}
	print(s);
}

function dump_var( name, value, indent ) {
	print_indent( indent );
	print( name + " = { \n");
	for( e in value ) {
		switch( typeof(value[e] ) ) {
			case 'object': {
				dump_var( e, value[e], indent+1 );
			}break;
			case 'string': {
				print_indent( indent+1 );
				print( e+" = \""+value[e]+"\"\n" );
			}break;
			default: {
				print_indent( indent+1 );
				print( e+" = "+value[e]+"\n" );
			}
		}
	}
	print_indent( indent );
	print("}, \n");
}

function test() {
	cloud.print("===== cloud print test =========\n");
	dump_var( "http", http, 0 );	
}

print("<pre>");

test();

print("</pre>");
