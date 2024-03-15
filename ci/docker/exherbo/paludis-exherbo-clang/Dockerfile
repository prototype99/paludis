FROM heirecka/exherbo-x86_64-pc-linux-gnu-gcc-with-clang:14
MAINTAINER Heiko Becker <heirecka@exherbo.org>

# Build generic binaries
COPY ./config/bashrc /etc/paludis/bashrc

COPY ./config/options/* /etc/paludis/options.conf.d/
COPY ./config/sets/paludis-deps.conf /etc/paludis/sets/

RUN chgrp tty /dev/tty \
    && eclectic env update \
    && source /etc/profile \
    && cave resolve \
            --execute \
            --preserve-world \
            --lazy \
            --keep-targets if-same \
            --keep if-same \
            --permit-old-version "*/*" \
            --recommendations ignore \
            --suggestions ignore \
            paludis-deps
RUN eclectic c++ set clang \
    && eclectic cc set clang \
    && eclectic cpp set clang

# Clean up
RUN rm -fr \
        /var/log/paludis/* \
        /var/cache/paludis/distfiles/* \
        /var/tmp/paludis/build/*

# Add non-privileged user
RUN useradd -M builder
USER builder
