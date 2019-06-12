(define-module (ggg forms))
(use-modules (rnrs io ports)
			 (ggg types))

(define input-string
  (lambda (prompt regex)
	(put-string (current-output-port) prompt)
	(let ((value (get-line (current-input-port))))
	  (if (regexp-exec regex value) value
		  (begin (newline) (input-string prompt regex))))))

(define input-password
  (lambda (prompt entropy)
	(put-string (current-output-port) prompt)
	(let ((value (get-password (current-input-port))))
	  (if (check-password value entropy) value
		  (begin (newline) (input-password prompt entropy))))))

;; export all symbols
(module-map
  (lambda (sym var)
	(module-export! (current-module) (list sym)))
  (current-module))

