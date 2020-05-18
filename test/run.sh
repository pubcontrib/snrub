#!/bin/sh
program=$1

if [ -z "$program" ]
then
    printf "Script failed! No program path given.\n"
    exit 1
fi

source test/suite/type/null.sh
source test/suite/type/number.sh
source test/suite/type/string.sh

source test/suite/operator/comment.sh
source test/suite/operator/value.sh
source test/suite/operator/assign.sh
source test/suite/operator/add.sh
source test/suite/operator/subtract.sh
source test/suite/operator/multiply.sh
source test/suite/operator/divide.sh

source test/suite/whitespace.sh
source test/suite/expression.sh
