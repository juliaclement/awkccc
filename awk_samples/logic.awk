BEGIN {
    do {
        while( a > 1 ) {
            if( a % 3 == 0 )
                print "triple"
            else
                print "Null";
            a -= 1;
        }
    }
    while ( a >= 0 );
}