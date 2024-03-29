. test/assert.sh

hint 'operator/warp'

# Arguments Warp
pass '("()<-" "DO" "(\"@>>\") (\"@\")") ("DO" #100# #200# #300#)' '#200#'
pass '("()<-" "DO" "(\"@>>\") (\"@>>\") (\"@\")") ("DO" #100# #200# #300#)' '#300#'
fail '("()<-" "DO" "(\"@>>\") (\"@>>\") (\"@>>\") (\"@\")") ("DO" #100# #200# #300#)' '#4#'
pass '("@>>")' '?'
pass '("()<-" "DO" "(\"@>>\")") ("DO")' '?'
pass '("()<-" "DO" "(\"@>>\") (\"@\")")
("x<-" "score" #2#)
("DO"
    ("x<-" "score" ("*" ("x->" "score") #3#))
    ("x<-" "score" ("*" ("x->" "score") #5#))
    ("x<-" "score" ("*" ("x->" "score") #7#))
)
("x->" "score")' '#10#'
pass '("()<-" "DO" "(\"@>>\") (\"@>>\") (\"@\")")
("x<-" "score" #2#)
("DO"
    ("x<-" "score" ("*" ("x->" "score") #3#))
    ("x<-" "score" ("*" ("x->" "score") #5#))
    ("x<-" "score" ("*" ("x->" "score") #7#))
)
("x->" "score")' '#14#'
pass '("()<-" "DO" "(\"@>>\") (\"@\") (\"@>>\")")
("x<-" "score" #2#)
("DO"
    ("x<-" "score" ("*" ("x->" "score") #3#))
    ("x<-" "score" ("*" ("x->" "score") #5#))
    ("x<-" "score" ("*" ("x->" "score") #7#))
)
("x->" "score")' '#10#'
pass '("()<-" "DO" "(\"@\") (\"@>>\") (\"@>>\")")
("x<-" "score" #2#)
("DO"
    ("x<-" "score" ("*" ("x->" "score") #3#))
    ("x<-" "score" ("*" ("x->" "score") #5#))
    ("x<-" "score" ("*" ("x->" "score") #7#))
)
("x->" "score")' '#6#'
