. test/assert.sh

hint 'operator/hash'

# Value Hashing
pass '("::" ?)' '#0#'
pass '("::" ##)' '#0#'
pass '("::" #100#)' '#100#'
pass '("::" #-100#)' '#-100#'
pass '("::" #662802839#)' '#662802839#'
pass '("::" #999999998#)' '#999999998#'
pass '("::" #999999999#)' '#999999999#'
pass '("::" #-999999998#)' '#-999999998#'
pass '("::" #-999999999#)' '#-999999999#'
pass '("::" "")' '#0#'
pass '("::" "word")' '#444#'
pass '("::" "100")' '#145#'
pass '("::" "The wicked soup officiates the competition.")' '#4150#'
pass '("::" "TTvo8DqrYbo90sIP")' '#1419#'
pass '("::" "52f01e93-a547-42a8-9fea-a8e74fc2951f")' '#2433#'
pass '("::" [])' '#0#'
pass '("::" [#1# #2# #3#])' '#6#'
pass '("::" [#100#])' '#100#'
pass '("::" [#100# #100#])' '#200#'
pass '("::" [#999999999# #1#])' '#-999999999#'
pass '("::" [#-999999999# #-1#])' '#999999999#'
pass '("::" [#999999999# #1# #1# #1#])' '#-999999997#'
pass '("::" [#-999999999# #-1# #-1# #-1#])' '#999999997#'
pass '("::" [#999999999# #3#])' '#-999999997#'
pass '("::" [#-999999999# #-3#])' '#999999997#'
pass '("::" [#999999999# #999999999# #999999999# #1#])' '#999999999#'
pass '("::" [#-999999999# #-999999999# #-999999999# #-1#])' '#-999999999#'
pass '("::" ["100"])' '#145#'
pass '("::" ["100" "100"])' '#290#'
pass '("::" [? #662802839# "The wicked soup officiates the competition." ["100" #100#]])' '#662807234#'
pass '("::" {})' '#0#'
pass '("::" {"" ?})' '#0#'
pass '("::" {"" #100#})' '#100#'
pass '("::" {"40" ?})' '#100#'
pass '("::" {"key" #100#})' '#429#'
pass '("::" {"key" "value"})' '#870#'
pass '("::" {"one" #1# "two" #2# "three" #3#})' '#1210#'
pass '("::" {"three" #3# "two" #2# "one" #1#})' '#1210#'
pass '("::" {"one" #999999999# "two" #999999999# "three" #999999999#})' '#-999998797#'
pass '("::" {"null" ? "number" #662802839# "string" "The wicked soup officiates the competition." "list" ["100" #100#]})' '#662809433#'
pass '("=" ("::" ?) ("::" ?))' '#1#'
pass '("=" ("::" ##) ("::" ##))' '#1#'
pass '("=" ("::" "") ("::" ""))' '#1#'
pass '("=" ("::" []) ("::" []))' '#1#'
pass '("=" ("::" {}) ("::" {}))' '#1#'

# Argument Error
fail '("::")' '#4#'

# Error Propogation
fail '("::" ("<>" "ERROR"))' '"ERROR"'
