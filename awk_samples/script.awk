#! /usr/bin/awk
BEGIN	{lb=0;}
$0~/^[A-Z][A-Z]*$/	{if (!lb) print "";
			printf( "@");}
	{lb = NF == 0;
	print $0}
