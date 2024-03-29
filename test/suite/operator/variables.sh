. test/assert.sh

hint 'operator/variables'

# Variable Roster
pass '("x[]")' '[]'
pass '("x<-" "a" ?) ("x[]")' '["a"]'
pass '("x<-" "a" #1#) ("x[]")' '["a"]'
pass '("x<-" "a" #1#) ("x<-" "b" #2#) ("x[]")' '["a" "b"]'
pass '("x<-" "b" #1#) ("x<-" "a" #2#) ("x[]")' '["a" "b"]'
pass '("x<-" "cba" #1#)
("x<-" "cab" #2#)
("x<-" "abc" #3#)
("x<-" "acb" #4#)
("x<-" "bca" #5#)
("x<-" "bac" #6#)
("x[]")' '["abc" "acb" "bac" "bca" "cab" "cba"]'
pass '("x<-" "a" #1#) ("x<-" "a" #2#) ("x[]")' '["a"]'
pass '("x<-" "a" #1#) ("x<-" "a" ?) ("x[]")' '["a"]'
pass '("x<-" "a" #1#) ("()<-" "VARIABLES" "(\"x[]\")") ("VARIABLES")' '[]'
pass '("x<-" "a" #1#) ("()<-" "VARIABLES" "(\"x^\" \"a\") (\"x[]\")") ("VARIABLES")' '["a"]'
