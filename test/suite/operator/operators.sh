. test/assert.sh

hint 'operator/operators'

# Operator Roster
pass '("()[]")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "@__" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
pass '("()<-" "A" "") ("()<-" "B" "") ("()<-" "C" "") ("()[]")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "@__" "A" "B" "C" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
pass '("()<-" "+" "") ("()<-" "-" "") ("()<-" "*" "") ("()[]")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "@__" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
pass '("()--" "+") ("()--" "-") ("()--" "*") ("()[]")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "@__" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
pass '("()<-" "A" "") ("()<-" "B" "") ("()<-" "C" "") ("()<-" "OPS" "(\"()[]\")") ("OPS")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "@__" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
