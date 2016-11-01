AC_PREREQ(2.69)
AC_INIT([unrelyzer], [2.0])
AM_INIT_AUTOMAKE
AC_PROG_CC

M4_BINARY="`pwd`/binary/m4"
FLEX_BINARY="`pwd`/binary/flex"
BISON_BINARY="`pwd`/binary/bison"

M4_SOURCE="`pwd`/m4-1.4.17"
FLEX_SOURCE="`pwd`/flex-2.5.39"
BISON_SOURCE="`pwd`/bison-3.0.2"

M4_PREREQ="m4 (GNU M4) 1.4.17"
FLEX_PREREQ="flex 2.5.35"
BISON_PREREQ="bison (GNU Bison) 3.0.2"

FLEX=""
BISON=""

AC_SUBST(FLEX)
AC_SUBST(BISON)


################# m4 #################
AC_CHECK_PROG(M4_CHECK, m4, yes, no)
if test "$M4_CHECK" == "no"
then
  if test \( -x "$M4_BINARY/bin/m4" \) -a \( "`$M4_BINARY/bin/m4 --version | head -1`" \== "$M4_PREREQ" \)
  then
    AC_MSG_NOTICE([precompiled executable exists in "$M4_BINARY"])
  else
    AC_MSG_NOTICE([building m4 from the source])
  	mkdir -p $M4_BINARY
  	tar -xzf $M4_SOURCE".tar.gz"
  	pushd $M4_SOURCE
  	./configure --prefix=$M4_BINARY
  	make --silent && make --silent install
  	popd
  	rm -rf $M4_SOURCE
  fi
  export PATH=$PATH:$M4_BINARY/bin
fi
####################################

################# bison #################
AC_CHECK_PROG(BISON_CHECK, bison, yes, no)
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
########################################

################# flex #################
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
########################################

AC_OUTPUT([src/parser/Makefile])

rm config.*