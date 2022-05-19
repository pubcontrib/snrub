. test/assert.sh

hint 'operator/ripoff'

# Ripoff Variables
pass '("x<-" "var" "val") ("x^" "var") ("x->" "var")' '"val"'
pass '("x<-" "var" "val") ("()<-" "DO" "(\"x->\" \"var\")") ("DO")' '?'
pass '("x<-" "var" "val") ("()<-" "DO" "(\"x^\" \"var\") (\"x->\" \"var\")") ("DO")' '"val"'
pass '("x<-" "var" "val") ("()<-" "DO" "(\"x^\" \"var\") (\"x<-\" \"var\" \"updated\")") ("DO") ("x->" "var")' '"val"'
pass '("x<-" "var" "val") ("()<-" "DO" "(\"x^\" \"var\") (\"x<-\" \"var\" \"updated\")") ("DO") ("x<-" "var" "last") ("x->" "var")' '"last"'
pass '("x^" "var") ("x->" "var")' '?'

# Argument Error
fail '("x^" ?)' '#4#'
fail '("x^" ##)' '#4#'
fail '("x^" [])' '#4#'
fail '("x^" {})' '#4#'

# Error Propogation
fail '("x^" ("<>" "ERROR"))' '"ERROR"'
