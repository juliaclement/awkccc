BEGIN {
    for( varName=1; varName < 10; varName++ ) {
        print varName;
    }
    for( ; varName < 20; ) {
        varName += 5;
        print varName;
    }
}