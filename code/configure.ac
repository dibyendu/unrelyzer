AC_PREREQ(2.69)
AC_INIT([unrelyzer], [2.0])
AM_INIT_AUTOMAKE
AC_PROG_CC

FLEX_BINARY="`pwd`/binary/flex"
BISON_BINARY="`pwd`/binary/bison"
FLEX_SOURCE="`pwd`/flex-2.5.39"
BISON_SOURCE="`pwd`/bison-3.0.2"
FLEX_PREREQ="flex 2.5.35"
BISON_PREREQ="bison (GNU Bison) 3.0.2"
FLEX=""
BISON=""

AC_SUBST(FLEX)
AC_SUBST(BISON)

AC_CHECK_PROG(FLEX_CHECK, flex, yes, no)
if test "$FLEX_CHECK" == "yes"
then
  if test ! "`flex -V`" \< "$FLEX_PREREQ"
  then
    AC_MSG_NOTICE([flex exists in the system.])
    FLEX="flex"
  else
    FLEX_CHECK="no"
  fi
fi
if test "$FLEX_CHECK" == "no"
then
  FLEX="$FLEX_BINARY/bin/flex"
  if test \( -x "$FLEX_BINARY/bin/flex" \) -a \( ! "`$FLEX_BINARY/bin/flex -V`" \< "$FLEX_PREREQ" \)
  then
    AC_MSG_NOTICE([precompiled executable exists in "$FLEX_BINARY"])
  else
    AC_MSG_NOTICE([minimum requirement is "$FLEX_PREREQ" ... building from the source])
    mkdir -p $FLEX_BINARY
    tar -xzf $FLEX_SOURCE".tar.gz"
    pushd $FLEX_SOURCE
    ./configure --prefix=$FLEX_BINARY
    make --silent && make --silent install
    popd
    rm -rf $FLEX_SOURCE
  fi
fi

AC_CHECK_PROG(BISON_CHECK, bison, yes)
if test "$BISON_CHECK" == "yes"
then
  if test ! "`bison -V | head -1`" \< "$BISON_PREREQ"
  then
    AC_MSG_NOTICE([bison exists in the system.])
    BISON="bison"
  else
    BISON_CHECK="no"
  fi
fi
if test "$BISON_CHECK" == "no"
then
  BISON="$BISON_BINARY/bin/bison"
  if test \( -x "$BISON_BINARY/bin/bison" \) -a \( ! "`$BISON_BINARY/bin/bison -V | head -1`" \< "$BISON_PREREQ" \)
  then
    AC_MSG_NOTICE([precompiled executable exists in "$BISON_BINARY"])
  else
    AC_MSG_NOTICE([minimum requirement is "$BISON_PREREQ" ... building from the source])
    mkdir -p $BISON_BINARY
    tar -xzf $BISON_SOURCE".tar.gz"
    pushd $BISON_SOURCE
    ./configure --prefix=$BISON_BINARY
    make --silent && make --silent install
    popd
    rm -rf $BISON_SOURCE
  fi 
fi

AC_OUTPUT([src/parser/Makefile])

rm config.*