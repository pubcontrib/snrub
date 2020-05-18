#!/bin/sh
. test/assert.sh

hint 'type/number'

pass '##' '#0#'
pass '#0#' '#0#'
pass '#1#' '#1#'
pass '#10#' '#10#'
pass '#01#' '#1#'
pass '#-1#' '#-1#'
fail '#' '#1#'
fail '#1' '#1#'
fail '1#' '#1#'
fail '#word#' '#3#'
