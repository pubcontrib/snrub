. test/assert.sh

hint 'operator/ripoff'

# Ripoff Variables
pass '("x<-" "value" "first") ("x^" "value") ("x->" "value")' '"first"'
pass '("x<-" "value" "first") ("()<-" "DO" "(\"x->\" \"value\")") ("DO")' '?'
pass '("x<-" "value" "first") ("()<-" "DO" "(\"x^\" \"value\") (\"x->\" \"value\")") ("DO")' '"first"'
pass '("x<-" "value" "first") ("()<-" "DO" "(\"x^\" \"value\") (\"x<-\" \"value\" \"updated\")") ("DO") ("x->" "value")' '"first"'
pass '("x<-" "value" "first") ("()<-" "DO" "(\"x^\" \"value\") (\"x<-\" \"value\" \"updated\")") ("DO") ("x<-" "value" "last") ("x->" "value")' '"last"'
pass '("x^" "var") ("x->" "var")' '?'

# Argument Error
fail '("x^" ?)' '#4#'
fail '("x^" ##)' '#4#'
fail '("x^" [])' '#4#'
fail '("x^" {})' '#4#'

# Error Propogation
fail '("x^" ("<>" "ERROR"))' '"ERROR"'
