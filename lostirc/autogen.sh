#!/bin/sh

# build it all
libtoolize --copy --force && \
aclocal $ACLOCAL_FLAGS && \
autopoint --force && \
autoheader && \
automake --include-deps --add-missing --copy && \
autoconf

# in case automake generated errors
autoconf
