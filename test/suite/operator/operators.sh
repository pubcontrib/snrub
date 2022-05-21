. test/assert.sh

hint 'operator/operators'

# Operator Roster
pass '("()[]")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
pass '("()<-" "A" "") ("()<-" "B" "") ("()<-" "C" "") ("()[]")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "A" "B" "C" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
pass '("()<-" "+" "") ("()<-" "-" "") ("()<-" "*" "") ("()[]")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
pass '("()--" "+") ("()--" "-") ("()--" "*") ("()[]")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
pass '("()<-" "A" "") ("()<-" "B" "") ("()<-" "C" "") ("()<-" "OPS" "(\"()[]\")") ("OPS")' '["!" "\"" "#" "$--" "$->" "$<-" "$[]" "%" "&" "()--" "()<-" "()[]" "()^" "*" "+" "-" "..." "/" "::" ";" "<" "<>" "<|>" "=" ">" "><" "?" "@" "@>>" "[# #]" "[o]--" "[o]->" "[o]<-" "_" "o" "x--" "x->" "x<-" "x[]" "x^" "|" "| |"]'
