function inner()
{
print arg
}

function outer( arg )
{
	arg=arg+1
	print arg
	inner()
}

BEGIN {
	arg=1
	inner()
	outer(6)
	inner()
	print arg
}
