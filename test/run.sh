#!/bin/sh
PROGRAM=$1

if [ -z "$PROGRAM" ]
then
    printf '[ERROR] No program path given.\n' 1>&2
    exit 1
fi

. test/assert.sh

. test/suite/type/null.sh
. test/suite/type/number.sh
. test/suite/type/string.sh
. test/suite/type/list.sh
. test/suite/type/call.sh

. test/suite/operator/evaluate.sh
. test/suite/operator/memorize.sh
. test/suite/operator/forget.sh
. test/suite/operator/promote.sh
. test/suite/operator/demote.sh
. test/suite/operator/variables.sh
. test/suite/operator/operators.sh
. test/suite/operator/catch.sh
. test/suite/operator/throw.sh
. test/suite/operator/add.sh
. test/suite/operator/subtract.sh
. test/suite/operator/multiply.sh
. test/suite/operator/divide.sh
. test/suite/operator/modulo.sh
. test/suite/operator/and.sh
. test/suite/operator/or.sh
. test/suite/operator/not.sh
. test/suite/operator/conditional.sh
. test/suite/operator/loop.sh
. test/suite/operator/chain.sh
. test/suite/operator/less.sh
. test/suite/operator/greater.sh
. test/suite/operator/equal.sh
. test/suite/operator/sort.sh
. test/suite/operator/type.sh
. test/suite/operator/number.sh
. test/suite/operator/string.sh
. test/suite/operator/hash.sh
. test/suite/operator/represent.sh
. test/suite/operator/length.sh
. test/suite/operator/get.sh
. test/suite/operator/set.sh
. test/suite/operator/range.sh
. test/suite/operator/read.sh
. test/suite/operator/write.sh

conclude
