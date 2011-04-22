# This file is copyrighted under the BSD-license for buildsystem files of KDE
# copyright 2010, Patrick von Reth <patrick.vonreth@gmail.com>
#
#
# - Try to find the LIBFFTW3 library
# Once done this will define
#
#  LIBFFTW3_FOUND            Set to TRUE if LIBFFTW3 librarys and include directory is found
#  LIBFFTW3_INCLUDE_DIR      The libfftw3 include directory
#  LIBFFTW3_LIBRARY          The libfftw3 librarys

if(NOT LIBFFTW3_PRECISION)
  message(STATUS "Searching for LIBFFTW3, using default precision float")
  set(LIBFFTW3_PRECISION FLOAT)
endif(NOT LIBFFTW3_PRECISION)

find_path(LIBFFTW3_INCLUDE_DIR fftw3.h)

if(LIBFFTW3_PRECISION STREQUAL FLOAT)
  set(LIBFFTW3_PRECISION_SUFFIX f)
endif(LIBFFTW3_PRECISION STREQUAL FLOAT)

if(LIBFFTW3_PRECISION STREQUAL DOUBLE)
  set(LIBFFTW3_PRECISION_SUFFIX "")
endif(LIBFFTW3_PRECISION STREQUAL DOUBLE)

if(LIBFFTW3_PRECISION STREQUAL LDOUBLE)
  set(LIBFFTW3_PRECISION_SUFFIX l)
endif(LIBFFTW3_PRECISION STREQUAL LDOUBLE)

find_library(LIBFFTW3_LIBRARY NAMES fftw3${LIBFFTW3_PRECISION_SUFFIX} libfftw3${LIBFFTW3_PRECISION_SUFFIX}-3 fftw3${LIBFFTW3_PRECISION_SUFFIX}-3)

if(FIND_LIBFFTW3_VERBOSE)
  message(STATUS 
  "LIBFFTW3_PRECISION ${LIBFFTW3_PRECISION}, searched for fftw3${LIBFFTW3_PRECISION_SUFFIX} libfftw3${LIBFFTW3_PRECISION_SUFFIX}-3 fftw3${LIBFFTW3_PRECISION_SUFFIX}-3
    and found ${LIBFFTW3_LIBRARY}"
  )
endif(FIND_LIBFFTW3_VERBOSE)

if(LIBFFTW3_LIBRARY AND LIBFFTW3_INCLUDE_DIR)
    set(LIBFFTW3_FOUND TRUE)
    message(STATUS "Found libfftw3 ${LIBFFTW3_LIBRARY}")
else(LIBFFTW3_LIBRARY AND LIBFFTW3_PLUGIN_PATH)
    message(STATUS "Could not find libfftw3, get it http://www.fftw.org/")
endif(LIBFFTW3_LIBRARY AND LIBFFTW3_INCLUDE_DIR)