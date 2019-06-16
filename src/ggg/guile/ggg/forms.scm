(define-module (ggg forms))
(use-modules (rnrs io ports)
			 (ggg types))

(define (passwd-exists name)
  (catch #t
		 (lambda () (getpwnam name))
		 (lambda (key . parameters) #f)))

(define (group-exists name)
  (catch #t
		 (lambda () (getgrnam name))
		 (lambda (key . parameters) #f)))

(define-public (input-value prompt read validate)
  (put-string (current-output-port) prompt)
  (let ((value (read (current-input-port))))
	(if (validate value) value (input-value prompt read validate))))

(define-public (input-regex prompt regex)
  (input-value prompt
			   get-line
			   (lambda (value) (regexp-exec regex value))))

(define-public (input-password prompt entropy)
  (input-value prompt
			   get-password
			   (lambda (value) (check-password value entropy))))

(define-public (input-username prompt regex)
  (input-value prompt
			   get-line
			   (lambda (value) (and (regexp-exec regex value)
									(not (ggg-entity-exists value))
									(not (passwd-exists value))))))

(define-public (validate-username value regex)
  (and (regexp-exec regex value)
	   (not (ggg-entity-exists value))
	   (not (passwd-exists value))
	   (not (group-exists value))))

(define-public (validate-password value entropy)
			   (check-password value entropy))
