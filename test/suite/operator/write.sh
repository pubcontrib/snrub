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
fail '("[o]<-" ("#" "text") "text")' '#3#'
fail '("[o]<-" "file.txt" ("#" "text"))' '#3#'
