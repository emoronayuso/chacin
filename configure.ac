# Datos principales para generar el configure
AC_INIT([chacin], [1.0], [emoronayuso@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/chacin.c])

# Comprueba si tenemos el compilador C (GNU CC)
AC_PROG_CC

# Comprueba las cabeceras
AC_CHECK_HEADERS([signal.h event.h getopt.h sys/socket.h netinet/in.h unistd.h arpa/inet.h sys/types.h stdlib.h])

# Archivos de configuracion de Makefile
AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT
