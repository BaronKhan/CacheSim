#!/bin/bash



# Folder name of cache to be tested

CACHE=multi-word-block;



# Note, filename must be consistent with folder name
INPUT=${CACHE}.input;

OUTPUT=${CACHE}.output;

PARAMS=${CACHE}.params;

GOT=${CACHE}.got;




cat ${CACHE}/${INPUT}  |  ./mem_sim $(< ${CACHE}/${PARAMS}) > ${CACHE}/${GOT}


# Look at the differences
diff -b -I '^#' -I '^ #' ${CACHE}/${GOT} ${CACHE}/${OUTPUT}