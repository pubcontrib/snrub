. test/assert.sh

hint 'operator/write'

# Argument Error
fail '("[o]<-")' '#4#'
fail '("[o]<-" ?)' '#4#'
fail '("[o]<-" ? ?)' '#4#'
fail '("[o]<-" ? ##)' '#4#'
fail '("[o]<-" ? "")' '#4#'
fail '("[o]<-" ? [])' '#4#'
fail '("[o]<-" ? {})' '#4#'
fail '("[o]<-" ##)' '#4#'
fail '("[o]<-" ## ?)' '#4#'
fail '("[o]<-" ## ##)' '#4#'
fail '("[o]<-" ## "")' '#4#'
fail '("[o]<-" ## [])' '#4#'
fail '("[o]<-" ## {})' '#4#'
fail '("[o]<-" "")' '#4#'
fail '("[o]<-" "" ?)' '#4#'
fail '("[o]<-" "" ##)' '#4#'
fail '("[o]<-" "" [])' '#4#'
fail '("[o]<-" "" {})' '#4#'
fail '("[o]<-" [])' '#4#'
fail '("[o]<-" [] ?)' '#4#'
fail '("[o]<-" [] ##)' '#4#'
fail '("[o]<-" [] "")' '#4#'
fail '("[o]<-" [] [])' '#4#'
fail '("[o]<-" [] {})' '#4#'
fail '("[o]<-" {})' '#4#'
fail '("[o]<-" {} ?)' '#4#'
fail '("[o]<-" {} ##)' '#4#'
fail '("[o]<-" {} "")' '#4#'
fail '("[o]<-" {} [])' '#4#'
fail '("[o]<-" {} {})' '#4#'

# Error Propogation
fail '("[o]<-" ("<>" "ERROR") "text")' '"ERROR"'
fail '("[o]<-" "file.txt" ("<>" "ERROR"))' '"ERROR"'
