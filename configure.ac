AC_PREREQ(2.69)
AC_INIT([Control_Flow_Graph], [1.0])
AM_INIT_AUTOMAKE
AC_PROG_CC

FLEX_BINARY="`pwd`/binary/flex"
FLEX_SOURCE="`pwd`/flex-2.5.39"
BISON_BINARY="`pwd`/binary/bison"
BISON_SOURCE="`pwd`/bison-3.0"
FLEX=""
BISON=""

AC_SUBST(FLEX)
AC_SUBST(BISON)

AC_CHECK_PROG(FLEX_CHECK, flex, yes)
if test x"$FLEX_CHECK" == x"yes" ; then
  AC_MSG_NOTICE([found flex in the system.])
  FLEX="flex"
else
  AC_MSG_NOTICE([building flex from source ...])
  mkdir -p $FLEX_BINARY
  tar -xzf $FLEX_SOURCE".tar.gz"
  pushd $FLEX_SOURCE
  ./configure --prefix=$FLEX_BINARY
  make --silent && make --silent install
  popd
  rm -rf $FLEX_SOURCE
  FLEX="$FLEX_BINARY/bin/flex"
fi

AC_CHECK_PROG(BISON_CHECK, bison, yes)
if test x"$BISON_CHECK" == x"yes" ; then
  AC_MSG_NOTICE([found bison in the system.])
  BISON="bison"
else
  AC_MSG_NOTICE([building bison from source ...])
  mkdir -p $BISON_BINARY
  tar -xzf $BISON_SOURCE".tar.gz"
  pushd $BISON_SOURCE
  ./configure --prefix=$BISON_BINARY
  make --silent && make --silent install
  popd
  rm -rf $BISON_SOURCE
  BISON="$BISON_BINARY/bin/bison"
fi

AC_OUTPUT([src/Makefile])

rm config.*
