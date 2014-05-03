(def account1 (ref 100))
(def account2 (ref 0))

; to read the current value of a ref, use (deref refname):
(println (deref account1))
; prints 100

; @refname is equivalent to (deref refname)
(println @account2)
; prints 0

(defn transfer [amount from to]
    (dosync
       (alter from - amount)
       (alter to   + amount)))

(println (transfer 100 account1 account2))
; prints 100


(defn log-deposit [account amount]
     (dosync
        (println "Depositing $" amount " into account, balance now: "
            (commute account + amount))))

(log-deposit account1 100)
(log-deposit account1 50)
(println @account1)

; (as good as) equivalent to 
;(log-deposit myaccount 50)
;(log-deposit myaccount 100) 
