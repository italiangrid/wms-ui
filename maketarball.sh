#!/bin/sh
\rm -f /tmp/glite-wms-api$1-$2.$3.tar.gz

tar czvf /tmp/glite-wms-api-$1-$2.$3.tar.gz \
            --exclude CMakeCache.txt \
            --exclude cmake_install.cmake \
            --exclude "*$4*" \
            --exclude "*Makefile" \
            --exclude "*CMakeFiles*" \
            --exclude install_manifest.txt \
            --exclude "*libglite*.so*" \
            --exclude "*pkgconfig/wms-api.pc" \
            --exclude "*/.git*" \
            --exclude "rpmbuild*" \
	    --exclude "usr*" \
	    --exclude "opt*" \
	    --exclude "etc*" \
            -C $5 \
            .
