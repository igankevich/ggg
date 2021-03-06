(use-modules (rnrs io ports)
			 (ggg types)
			 (ggg forms)
			 (oop goops))

(setlocale LC_ALL "")
(define re-name (make-regexp "^[a-zа-я '\\-]+$" regexp/icase))
(define re-username (make-regexp "^[a-z]([a-z0-9_\\-]|\\.){1,16}[a-z0-9]$" regexp/icase))

(let ((first-name (input-regex "First name: " re-name))
	  (last-name (input-regex "Last name: " re-name))
	  (username (input-username "Username: " re-username))
	  (password (input-password "Password: " 30.0))
	  (date (localtime (current-time))))
  (begin
	;; calculate expiration date
	(set-tm:sec date 0)
	(set-tm:min date 0)
	(set-tm:hour date 0)
	(set-tm:mday date 1)
	(if (< (tm:mon date) 6)
		(set-tm:mon date 6)
		(begin
		  (set-tm:mon date 1)
		  (set-tm:year date (+ (tm:year date) 1))))
	(with-transaction
	  (flags %entities %accounts) %read-write
	  (lambda (store)
		(store-add
		  store
		  (make <entity>
				#:name username
				#:description (string-join (list last-name first-name) " ")
				#:home (string-join (list "/home/" username) "")
				#:shell "/bin/bash"))
		(store-add
		  store
		  (make <account>
				#:name username
				#:password (password-hash password)
				#:expiration-date (car (mktime date))))))))
