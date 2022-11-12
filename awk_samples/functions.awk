function inner()
{
	a=arg+2
	print( arg)
}

function outer( arg )
{
	arg=arg+1
	print( arg)
	inner(arg)
}

BEGIN {
	arg=1
	inner(2)
	outer(6)
	inner(3)
	print( arg)
}
