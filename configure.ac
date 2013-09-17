AC_INIT(test, 0.1, bca)

dnl see if testing needs to take place
HAVE_CWD=-1
DO_TEST=1
./bca --host $HOST_IDENTIFIER --checkvalue TESTRESULTS HAVE_CWD
if @<:@ $? == 2 @:>@
then
	DO_TEST=0
	HAVE_CWD=1
else
	./bca --host $HOST_IDENTIFIER --checkvalue TESTRESULTS DONT_HAVE_CWD
	if @<:@ $? == 2 @:>@
	then
		HAVE_CWD=0
		DO_TEST=0
	fi
fi

dnl perform testing and cache result
if @<:@ $DO_TEST == 1 @:>@
then
	AC_CHECK_FUNC(getcwd, [ HAVE_CWD=1], [ HAVE_CWD=0])

	if @<:@ $HAVE_CWD == 1 @:>@
	then
		./bca --host $HOST_IDENTIFIER --smartaddvalue TESTRESULTS HAVE_CWD
	fi

	if @<:@ $HAVE_CWD == 0 @:>@
	then
		./bca --host $HOST_IDENTIFIER --smartaddvalue TESTRESULTS DONT_HAVE_CWD
	fi
fi

dnl take test results (or cached value) and manipulate build configureation
if @<:@ $HAVE_CWD == 1 @:>@
then
	./bca --host $HOST_IDENTIFIER --smartaddvalue CFLAGS -DHAVE_CWD
fi


