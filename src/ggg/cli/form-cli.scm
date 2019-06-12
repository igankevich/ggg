(use-modules (rnrs io ports)
			 (ggg types)
			 (ggg forms)
			 (oop goops))

(setlocale LC_ALL "")
(define re-name (make-regexp "^[a-zа-я ]+$" regexp/icase))
(define re-username (make-regexp "^[a-z]([a-z0-9_\\-]|\\.){1,16}[a-z0-9]$" regexp/icase))

(let ((first-name (input-string "First name: " re-name))
	  (last-name (input-string "Last name: " re-name))
	  (username (input-string "Username: " re-username))
	  (password (input-password "Password: " 30.0)))
  (begin
	(ggg-entity-insert
	  (make <entity>
			#:name username
			#:real-name (string-join (list last-name first-name) " ")
			#:home-directory (string-join (list "/home/" username))
			#:shell "/bin/bash"))
	(ggg-account-insert
	  (make <account>
			#:name username
			#:expiration-date 0))))
